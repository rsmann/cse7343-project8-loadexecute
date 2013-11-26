// Scott Mann - CSE 7343 - 11/26/2013
// A simple shell to go with the kernel.

// Function Prototypes
void displayError();
int matches(char*, char*);

// Main processing loop for the shell
int main() {
	char input[255];
	char buffer[13312];
	char part[1][255];
	int i = 0;
	int partIndex = 0;
	int offset = 0;

	interrupt(0x21, 0, "Shell Version 0.2.234\r\n\0", 0, 0);

	// Begin the command processing loop
	while(1)
	{
		partIndex = 0;
		offset = 0;

		// Capture command line input
		interrupt(0x21, 0, "Shell> ", 0, 0);
		interrupt(0x21, 1, input, 0, 0);
		interrupt(0x21, 0, "\r\n\0", 0, 0);

		for (i = 0; i < 255; i++)
		{
			if (input[i] == '\0' || input[i] == '\r')
			{
				break;
			}
			else if (input[i] == ' ')
			{
				partIndex++;
				offset = 0;
				continue;
			}

			part[partIndex][offset] = input[i];
			offset++;
		}

		if (matches("execute", part[0]) == 1)
		{
			interrupt(0x21, 0, "Execute!\r\n\0", 0, 0);
			interrupt(0x21, 0x6, part[1], 0x2000, 0);
		}
		else if (matches("type", part[0]) == 1)
		{
			interrupt(0x21, 0, "Type!\r\n\0", 0, 0);
			interrupt(0x21, 0x3, part[1], buffer, 0);
			interrupt(0x21, 0x0, buffer, 0, 0);
		}
		else
		{
			interrupt(0x21, 0, "Error!\r\n\0", 0, 0);
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
		maxSize = size1;
	}
	else
	{
		maxSize = size2;
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
