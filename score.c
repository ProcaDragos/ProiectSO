#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	char tresureid[40];
	char name[40];
	float x;
	float y;
	char text[100];
	int val;
} tr;

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, " %s <hunt_dir>\n", argv[0]);
		return 1;
	}

	char path[256];
	snprintf(path, sizeof(path), "%s/comoara.bin", argv[1]);

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	tr t;
	struct {
		char name[40];
		int score;
	} users[100];

	int count = 0;

	while (read(fd, &t, sizeof(tr)) == sizeof(tr)) {
		if (strlen(t.name) == 0) continue;

		int f = 0;
		for (int i = 0; i < count; ++i) {
			if (strcmp(users[i].name, t.name) == 0) {
				users[i].score += t.val;
				f= 1;
				break;
			}
		}

		if (!f) {
			strcpy(users[count].name, t.name);
			users[count].score = t.val;
			count++;
		}
	}

	close(fd);

	printf("rezultate '%s':\n", argv[1]);
	for (int i = 0; i < count; ++i) {
		printf("%s: %d\n", users[i].name, users[i].score);
	}

	return 0;
}
