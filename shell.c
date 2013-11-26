// Scott Mann - CSE 7343 - 10/20/2013
// A simple Unix shell.

// Function Prototypes
void displayError();
int isType(char* input);

// Main processing loop for the shell
int main() {
	char input[255];
	char buffer[13312];
	char part[1][255];
	int i = 0;
	int partIndex = 0;
	int offset = 0;

	interrupt(0x21, 0, "Shell Version 0.1.234\r\n\0", 0, 0);

	// Begin the command processing loop
	while(1)
	{
		// Capture command line input
		interrupt(0x21, 0, "Shell> ", 0, 0);
		interrupt(0x21, 1, input, 0, 0);
		interrupt(0x21, 0, "\r\n\0", 0, 0);

		for (i = 0; i < 255; i++)
		{
			if (input[i] == ' ')
			{
				partIndex++;
				offset = 0;
				continue;
			}
			else if (input[i] == '\0')
			{
				break;
			}

			part[partIndex][offset] = input[i];
			offset++;
		}

		if (matches("execute", part[0]) == 1)
		{
			interrupt(0x21, 0x6, part[1], 0, 0);
		}
		else if (matches("type", part[0]) == 1)
		{
			//interrupt(0x21, 0, "Type!\r\n\0", 0, 0);
			interrupt(0x21, 0x3, part[1], buffer, 0);
			interrupt(0x21, 0x0, buffer, 0, 0);
		}
		else
		{
			displayError();
		}

	}

	return 0;
}

int matches(char* s1, char* s2)
{
	int match = 1;
	int n = 0;
	int size1 = sizeof(s1);
	int size2 = sizeof(s2);
	int maxSize = 0;

	if (size2 >= size1)
	{
		maxSize = size2;
	}
	else
	{
		maxSize = size1;
	}

	interrupt(0x21, 0, s1, 0, 0);
	interrupt(0x21, 0, "\r\n\0", 0, 0);
	interrupt(0x21, 0, s2, 0, 0);
	interrupt(0x21, 0, "\r\n\0", 0, 0);

    //Compare the name
    for (n = 0; n < maxSize; n++)
    {
        if(s1[n] != s2[n])
        {
        	interrupt(0x21, 0, "No match\r\n\0", 0, 0);
            return 0;
        }
    }

	interrupt(0x21, 0, "Match\r\n\0", 0, 0);
    return 1;
}

// Display the erroneous command back to the user
void displayError()
{
	interrupt(0x21, 0, "Bad command!\r\n\0", 0, 0);
}

// Execute the command passed in
//void execute(char* command)
//{
//	char args[1][1];
//	args[0][0] = '\0';
//
//	int pid = fork();
//
//	if (pid == 0)
//	{
//		printf("Executing...\n");
//		execvp(command, args);
//	}
//	else
//	{
//		wait(pid);
//		return;
//	}
//}

// Is the command "exit"
//bool isExit(char* command)
//{
//	return !strcmp(command, "exit");
//}

// Is the command "type"
//bool isType(char* command)
//{
//	return !strcmp(command, "type");
//}

// Output the file to the screen byte by byte
//void type(char *filename)
//{
//	printf("Opening %s for reading...\n", filename);
//
//	char currentChar = 0;
//	FILE *handle;
//
//	// Open the file
//	handle = fopen(filename, "r");
//
//	// If we didn't get a valid handle back, an error occurred
//	if(handle == NULL)
//	{
//		perror("Error while opening the file.\n");
//		return;
//	}
//
//	// Read every character and print it to the screen
//	while((currentChar = fgetc(handle)) != EOF)
//	{
//		printf("%c", currentChar);
//	}
//
//	// Close the file cleanly
//	fclose(handle);
//
//	return;
//}
