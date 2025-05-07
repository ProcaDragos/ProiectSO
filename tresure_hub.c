#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>


pid_t p = 0;
int run = 0;
char huntid[6], id[5];

void list_hunts() {
    DIR *d = opendir(".");
    if (!d) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[256];
    int count;
    printf("nr hunts:\n");

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") == 0)
            continue;

        if (entry->d_type == DT_DIR) {

            int written = snprintf(path, sizeof(path), "%.*s/comoara.bin", (int)(sizeof(path) - 11), entry->d_name);
            if (written >= sizeof(path)) {
                fprintf(stderr, "Path too long: %s\n", entry->d_name);
                continue;
            }

            int fd = open(path, O_RDONLY);
            if (fd < 0) {

                continue;
            }

            count = 0;
            char buf[168];
            ssize_t r;
            while ((r = read(fd, buf, sizeof(buf))) == sizeof(buf)) {
                count++;
            }

            if (r < 0) {
                perror("read");
            }

            close(fd);
            printf("Hunt: %s, Count: %d\n", entry->d_name, count);
        }
    }

    closedir(d);
}

void list_treasures(int sig) {
    printf("\nEnter huntid: ");
    if (scanf("%5s", huntid) != 1) {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        return;
    }

    if (child == 0) {
        execlp("./main", "main", "--list", huntid, (char *)NULL);
        perror("execlp");
        _exit(1);
    } else {
        waitpid(child, NULL, 0);
    }
}

void view_treasure(int sig) {
    printf("\nEnter huntid: ");
    if (scanf("%5s", huntid) != 1) {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    printf("Enter id: ");
    if (scanf("%4s", id) != 1) {
        fprintf(stderr, "Invalid input\n");
        return;
    }

    char command[256];
    int written = snprintf(command, sizeof(command), "./main --view %s %s", huntid, id);
    if (written >= sizeof(command)) {
        fprintf(stderr, "Command too long.\n");
        return;
    }

    int status = system(command);
    if (status == -1) {
        perror("system");
    }
}

void stop_monitor(int sig) {
    exit(0);
}

void monitor_procces() {
    struct sigaction sa = {0};
    sa.sa_flags = SA_RESTART;

    sa.sa_handler = list_hunts;
    sigaction(SIGUSR1, &sa, NULL);

    sa.sa_handler = list_treasures;
    sigaction(SIGUSR2, &sa, NULL);

    sa.sa_handler = view_treasure;
    sigaction(SIGILL, &sa, NULL);

    sa.sa_handler = stop_monitor;
    sigaction(SIGTERM, &sa, NULL);

    printf(" (PID %d)\n", getpid());
    while (1) pause();
}

int main(int argc, char **argv) {
    char m[50];

    while (1) {
        printf("Introducere comanda: ");
        fflush(stdout);

        if (scanf("%49s", m) != 1) {
            fprintf(stderr, "Eroare la citire comanda.\n");
            continue;
        }

        if (strcmp(m, "start_monitor") == 0) {
            if (run == 1) {
                printf("Monitorizarea ruleaza deja\n");
            } else {
                p = fork();
                if (p < 0) {
                    perror("fork");
                    exit(1);
                } else if (p == 0) {
                    monitor_procces();
                    exit(0);
                } else {
                    run = 1;
                }
            }
        } else if (strcmp(m, "list_hunts") == 0) {
            if (!run)
                printf("Monitorizarea nu ruleaza\n");
            else
                kill(p, SIGUSR1);

        } else if (strcmp(m, "list_treasures") == 0) {
            if (!run)
                printf("Monitorizarea nu ruleaza\n");
            else
                kill(p, SIGUSR2);

        } else if (strcmp(m, "view_treasure") == 0) {
            if (!run)
                printf("Monitorizarea nu ruleaza\n");
            else
                kill(p, SIGILL);

        } else if (strcmp(m, "stop_monitor") == 0) {
            if (!run) {
                printf("Monitorizarea nu ruleaza\n");
            } else {
                kill(p, SIGTERM);
                waitpid(p, NULL, 0);
                run = 0;
                printf("Monitorizarea oprit\n");
            }
        } else if (strcmp(m, "exit") == 0) {
            if (run) {
                printf("Monitorizarea ruleaza\n");
            } else {
                exit(0);
            }
        } else {
            printf("Comanda necunoscuta\n");
        }
    }

    return 0;
}

