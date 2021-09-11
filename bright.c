#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define MODE_ABSOLUTE 0
#define MODE_REL_ADD 1
#define MODE_REL_SUB 2

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("%s", "Usage: bright %|+%|-%\n\n");
		printf("%s", "VAL must be a percentage. %=100 means the maximum brightness supported by the video driver, and %=0 means the minimum.\n");
		printf("%s", "+/- specifies relative change.\n");
		printf("%s", "    EXAMPLE: \n");
		printf("%s", "    bright 25    : set brightness to 25%\n");
		printf("%s", "    bright -10   : set brightness 10% lower\n");
		return 1;
	}

	// check if first char of the first param is '+'; if it is,
	// then we should ADD the percentage to the current brightness
	// value later on. If it is '-', then we should subtract.
	// If it isn't any of that, then we should just interpret
	// this parameter as an absolute percentage.
	int mode;
	int percentage;

	if (argv[1][0] == '+') {
		// pointer hackery: start counting after the first character
		percentage = atoi(&argv[1][1]);
		mode = MODE_REL_ADD;
	} else if (argv[1][0] == '-') {
		percentage = atoi(&argv[1][1]);
		mode = MODE_REL_SUB;
	} else {
		percentage = atoi(&argv[1][0]);
		mode = MODE_ABSOLUTE;
	}

	char *basePath = "/sys/class/backlight/";
	DIR *d;
	struct dirent *dir;

	d = opendir(basePath);
	if (d) {
		// iterate over each subdirectory in basePath
		while ((dir = readdir(d)) != NULL) {
			// ignore "." and ".." directories
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
				continue;

			// open brightness file
			char maxBrightPath[128];
			char brightPath[128];

			// concatenate path names
			strcpy(maxBrightPath, basePath);
			strcat(maxBrightPath, dir->d_name);
			strcat(maxBrightPath, "/max_brightness");

			strcpy(brightPath, basePath);
			strcat(brightPath, dir->d_name);
			strcat(brightPath, "/brightness");

			FILE *mbf = fopen(maxBrightPath, "r");
			FILE *bf = fopen(brightPath, "r+");

			if (mbf == NULL) {
				printf("Error opening maximum brightness file descriptor.\n");
				return 1;
			} else if (bf == NULL) {
				printf("Error opening brightness file descriptor as read+write. Do you have root privileges?\n");
				return 1;
			}

			// determine file sizes
			long mbfSize;
			long bfSize;
			fseek(mbf, 0, SEEK_END);
			mbfSize = ftell(mbf);
			rewind(mbf);
			fseek(bf, 0, SEEK_END);
			bfSize = ftell(bf);
			rewind(bf);

			// allocate memory for storing the value
			char *maxBrightStr = malloc(mbfSize + 1);
			char *currBrightStr = malloc(bfSize + 1);
			fread(maxBrightStr, 1, mbfSize, mbf);
			fread(currBrightStr, 1, bfSize, bf);

			// convert string values to integer
			int maxBright = atoi(maxBrightStr);
			int currBright = atoi(currBrightStr);

			int percentageVal = percentage * maxBright / 100;
			int finalBrightness;

			// calculate final brightness based on mode
			if (mode == MODE_REL_ADD) {
				finalBrightness = currBright + percentageVal;
			} else if (mode == MODE_REL_SUB) {
				finalBrightness = currBright - percentageVal;
			} else {
				finalBrightness = percentageVal;
			}

			if (finalBrightness < 0) {
				finalBrightness = 0;
			} else if (finalBrightness > maxBright) {
				finalBrightness = maxBright;
			}

			// convert brightness integer to string
			char finalBrightnessStr[16];
			sprintf(finalBrightnessStr, "%d", finalBrightness);

			// write brightness to file
			fwrite(finalBrightnessStr, strlen(finalBrightnessStr), 1, bf);

			// free resources
			free(maxBrightStr);
			free(currBrightStr);

			fclose(mbf);
			fclose(bf);
		}

		closedir(d);
	} else {
		char errStr[128];
		strcpy(errStr, "Couldn't open directory ");
		strcat(errStr, basePath);
		printf("%s\n", errStr);
	}
}
