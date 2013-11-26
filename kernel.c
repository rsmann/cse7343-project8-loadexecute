/* Kernel.c
 * Scott Mann
 * CSE 7343 - Project 7
 * A simple kernel to support printing a string to video, 
 * reading a line from the keyboard, and reading a sector from disk.
 */

/* Function Prototypes */
int getRegisterValue(char rh, char rl);
void handleInterrupt21(int ax, int bx, int cx, int dx);
int mod(int a, int b);
int div(int a, int b);
void printString(char* message);
void readSector(char* buffer, int sector);
void readString(char* buffer);

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
char commandPrintCharacter = 0xE;

char deviceFloppy = 0;


char charBackspace = 0x8;
char charEnter = 0xD;
char charLineFeed = 0xA;
char charNull = 0x0;

/* The main entry point of the program */
int main()
{
	char inputBuffer[80];
	char sectorBuffer[512];
	char line[80];

	/* Step 1 Requirement */
	printString("Hello World\r\n\0");
	
	/* Step 2 Requirement */
	printString("Enter a line: \0");
	readString(inputBuffer);
	printString("\r\n\0");
	printString(inputBuffer);

	/* Step 3 Requirement */
	readSector(sectorBuffer, 30);
	printString(sectorBuffer);

	/* Step 4 Requirement */
	makeInterrupt21();
	/* Step 5 breaks this behavior */
	/* interrupt(interruptCustom, 0, 0, 0, 0); */

	/* Step 5 Requirement */
	/* makeInterrupt21 already executed in the step before */
	interrupt(interruptCustom, commandReadString, line, 0, 0);
	interrupt(interruptCustom, commandPrintString, line, 0, 0);
	interrupt(interruptCustom, 99, 0, 0, 0);

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
