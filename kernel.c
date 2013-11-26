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

/* Read a file from disk into the specified buffer */
void readFile(char* filename, char* buffer)
{
        char dir[sectorSize];
        int i = 0;
        int n = 0;
        int match = 0;
        int dirIndex = 0;
        int offset = 0;

        /* Load the directory sector into a 512 byte character array using readSector */
        readSector(dir, sectorDirectory);

        /* Go through the directory trying to match the filename. If not found, return. */
        for (i = 0; i < sectorSize; i = i + dirEntrySize)
        {
                match = 1;

                //Compare the name
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
                        /* Filename matches */
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
        /* Loading file sector by sector */

        for (i = dirIndex + maxFilenameLength; i < dirIndex + dirEntrySize; i++)
        {
                if (dir[i] == 0)
                {
                        /* Found null in sector list. Exiting. */
                        break;
                }

                /* Reading into buffer. */
                readSector(buffer + offset, dir[i]);
                offset += sectorSize;
        }

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