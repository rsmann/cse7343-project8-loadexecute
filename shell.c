// Scott Mann - CSE 7343 - 10/20/2013
// A simple Unix shell.

// Function Prototypes
void displayError();

// Main processing loop for the shell
int main() {
	char input[255];
	interrupt(0x21, 0, "Shell Version 0.1.234\r\n\0", 0, 0);

	// Begin the command processing loop
	while(1)
	{
		// Capture command line input
		interrupt(0x21, 0, "Shell> ", 0, 0);
		interrupt(0x21, 1, input, 0, 0);
		interrupt(0x21, 0, "\r\n\0", 0, 0);
		displayError();

	}

	return 0;
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
