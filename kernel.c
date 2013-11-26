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

/* Some pseudo-constants (since I'm unsure of K&R C requirements)
 * for use in calculations in other functions involving memory offsets
 */
char interruptVideo = 0x10;
char interruptDisk = 0x13;
char interruptKeyboard = 0x16;
char interruptCustom = 0x21;

char commandPrintString = 0x0;
char commandReadString = 0x1;
char commandReadSector = 0x2;
char commandReadFile = 0x3;
char commandPrintCharacter = 0xE;

char deviceFloppy = 0;


char charBackspace = 0x8;
char charEnter = 0xD;
char charLineFeed = 0xA;
char charNull = 0x0;

#define maxFileSize 13312
#define maxFilenameLength 6

int sectorMap = 1;
int sectorDirectory = 2;
#define sectorSize 512
#define dirEntrySize 0x20

/* The main entry point of the program */
int main()
{
	char buffer[maxFileSize];
	printString("Starting...\r\n\0");

	makeInterrupt21();
	
	printString("Reading file into buffer.\r\n\0");
	interrupt(interruptCustom, commandReadFile, "messag\0", buffer, 0); /* Read file into buffer */

	printString("Printing out the file.\r\n\0");
	interrupt(interruptCustom, commandPrintString, buffer, 0, 0); /* Print out the file */

	while (1) {}
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

/* Read a file from disk into the specified buffer */
void readFile(char* filename, char* buffer)
{
	char dir[sectorSize];
	int i = 0;
	int n = 0;
	int match = 0;
	int dirIndex = 0;
	int offset = 0;

	printString("Reading file.\r\n\0");
	printString(filename);
	printString("\r\n\0");

	/* Load the directory sector into a 512 byte character array using readSector */
	readSector(dir, sectorDirectory);

	/* Go through the directory trying to match the filename. If not found, return. */
	for (i = 0; i < sectorSize; i = i + dirEntrySize)
	{
		match = 1;

		//copy the name over
		for (n = 0; n < 6; n++)
		{
			if(filename[n] != dir[i + n])
			{
				match = 0;
				break;
			}
		}

		if (match == 1)
		{
			printString("Filename matches.\r\n\0");
			break;
		}
	}

	/* Did we ever find it? */
	if (i == sectorSize)
	{
		printString("File not found.\r\n\0");
		return;
	}

	/* Which directory index held the file? */
	dirIndex = i;
	offset = 0;

	/* Using the sector numbers in the directory, load the file, sector by sector, into the buffer array */
	printString("Loading file sector by sector.\r\n\0");

	for (i = dirIndex + maxFilenameLength; i < dirIndex + dirEntrySize; i++)
	{
		printString(dir[i]);

		if (dir[i] == 0)
		{
			printString("Found null in sector list. Exiting.\r\n\0");
			break;
		}

		printString("Reading into buffer.\r\n\0");
		readSector(buffer + offset, dir[i]);
		offset += sectorSize;

		printString(buffer);
	}

	/* Return */
	return;
}

/* Read a sector from disk */
void readSector(char* buffer, int sector)
{
	int relativeSector = mod(sector, 18) + 1;
	int head = mod(sector / 18, 2);
	int track = sector / 36;
	int sectorCount = 1;

	interrupt(interruptDisk, 
				getRegisterValue(commandReadSector, sectorCount), 
				buffer, 
				getRegisterValue(track, relativeSector), 
				getRegisterValue(head, deviceFloppy));

}

/* Read a line from the keyboard */
void readString(char* buffer)
{
	char currentChar = '\0';
	int charIndex = 0;

	/* Ensure that the input buffer is not zero-length */
	if (sizeof(buffer) ==0)
	{
		printString("Zero-length buffer exception.\r\0");
		return;
	}

	do
	{
		/* Read the first character from the keyboard to set up the loop */
		/* Interrupt returns the ASCII code for the key pressed. */
		currentChar = interrupt(interruptKeyboard, 0, 0, 0, 0);

		if (currentChar != charBackspace)
		{
			/* Output it to the screen */
			interrupt(interruptVideo, getRegisterValue(commandPrintCharacter, currentChar), 0, 0, 0);

			buffer[charIndex] = currentChar;
			charIndex++;
		}
		else if (currentChar == charBackspace)
		{
			if (charIndex > 0) charIndex--;
			{
				/* Clear the character from the screen instead of just backing the cursor up */
				interrupt(interruptVideo, getRegisterValue(commandPrintCharacter, charBackspace), 0, 0, 0);
				interrupt(interruptVideo, getRegisterValue(commandPrintCharacter, charNull), 0, 0, 0);
				interrupt(interruptVideo, getRegisterValue(commandPrintCharacter, charBackspace), 0, 0, 0);

				/* Clear the character from the buffer as well. */				
				buffer[charIndex] = charNull;
			}
		}

	} while (currentChar != charEnter);

	/* Append line feed and null to end of input */
	buffer[charIndex] = charLineFeed;
	buffer[charIndex + 1] = charNull;
}
