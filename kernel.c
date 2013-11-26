/* Kernel.c
 * Scott Mann
 * CSE 7343 - Project 8
 * A project to read files into memory and execute programs.
 * Also, a shell to execute other programs and print out ASCII files.
 */

/* Function Prototypes */
int getRegisterValue(char rh, char rl);
void handleInterrupt21(int ax, int bx, int cx, int dx);
int mod(int a, int b);
int div(int a, int b);
void printString(char* message);
void readSector(char* buffer, int sector);
void readString(char* buffer);
void readFile(char* filename, char* buffer);
void executeProgram(char* name, int segment);
void terminate();

/* Some pseudo-constants (since I'm unsure of K&R C requirements)
 * for use in calculations in other functions involving memory offsets
 */
#define interruptVideo 0x10
#define interruptDisk 0x13
#define interruptKeyboard 0x16
#define interruptCustom 0x21

#define commandPrintString 0x0
#define commandReadString 0x1
#define commandReadSector 0x2
#define commandReadFile 0x3
#define commandExecuteProgram 0x6
#define commandTerminate 0x7
#define commandPrintCharacter 0xE

#define deviceFloppy 0

#define charBackspace 0x8
#define charEnter 0xD
#define charLineFeed 0xA
#define charNull 0x0

#define maxFileSize 13312
#define maxFilenameLength 6

#define sectorMap 1
#define sectorDirectory 2
#define sectorSize 512
#define dirEntrySize 0x20

/* The main entry point of the program */
int main()
{
	char buffer[maxFileSize];

	makeInterrupt21();
	
	/* Step 1 Requirement */
	//interrupt(interruptCustom, commandReadFile, "messag\0", buffer, 0); /* Read file into buffer */
	//interrupt(interruptCustom, commandPrintString, buffer, 0, 0); /* Print out the file */

	/* Step 2 Requirement */
	//interrupt(interruptCustom, commandExecuteProgram, "tstprg\0", 0x2000, 0); /* Read file into buffer */

	/* Step 3 Requirement */
	interrupt(interruptCustom, commandExecuteProgram, "tstpr2\0", 0x2000, 0); /* Read file into buffer */

	while (1) {}
}

void executeProgram(char* name, int segment)
{
	char buffer[maxFileSize];
	int i = 0;

	/* Load file into buffer */
	printString("Loading file\r\n\0");
	interrupt(interruptCustom, commandReadFile, name, buffer, 0); /* Read file into buffer */

	//printString(buffer);

	printString("Copying buffer\r\n\0");
	/* Copy the buffer into the segment */
	for (i = 0; i < maxFileSize; i++)
	{
		putInMemory(segment, i, buffer[i]);
	}

	printString("Launching.\r\n\0");
	/* Launch the program */
	launchProgram(segment);
	printString("Launched.\r\n\0");

}

/* Handles any incoming interrupt 21 calls */
void handleInterrupt21(int ax, int bx, int cx, int dx)
{
	switch (ax)
	{
		case 0x0: /* commandPrintString */
			printString(bx);
			break;

		case 0x1: /* commandReadString */
			readString(bx);
			printString("\r\n\0");
			break;

		case 0x2: /* commandReadSector */
			readSector(bx, cx);
			break;

		case 0x3: /* commandReadFile */
			readFile(bx, cx);
			break;

		case 0x6: /* commandExecuteProgram */
			executeProgram(bx, cx);
			break;

		case 0x7: /* commandTerminate */
			terminate();
			break;

		default:
			printString("Bad interrupt call.\r\n\0");
	}
}

int mod(int a, int b)
{
	while (a >= b)
	{
		a = a - b;
	}

	return a;
}

int div(int a, int b)
{
	int quotient = 0;

	while (quotient * b < a)
	{
		quotient++;
	}

	return quotient;
}

/* Reusable function to calculate an integer value from a high byte and low byte*/
int getRegisterValue(char rh, char rl)
{
	return rh * 256 + rl;
}

/* Print a character array to the screen */
void printString(char* message)
{
	/* Grab the length of the message */
	int len = sizeof(message);
	char currentChar = '\0';
	int i = 0;

	/* If the message is empty, we're done. */
	if (len == 0) return;

	/* Read the first character to set up the loop */
	currentChar = message[i];

	/* Read each character and output it until we encounter \0 */
	while (currentChar != charNull)
	{
		interrupt(interruptVideo, getRegisterValue(commandPrintCharacter, currentChar), 0, 0, 0);
		i++;
		currentChar = message[i];
	}

	return;
}

void terminate()
{
	printString("Terminating...\r\n\0");
	while (1) {}
}

/* Read a file from disk into the