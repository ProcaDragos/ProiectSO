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
int pipefd[2];
int cmdpipe[2];

char huntid[20], id[20];

void tiparire(const char *msg) {
    write(pipefd[1], msg, strlen(msg));
}

void list_hunts() {
    DIR *d = opendir(".");
    if (!d) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char path[4096];
    int count = 0;
    char buffer[512];

    while ((entry = readdir(d)) != NULL) {
        count = 0;
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(path, sizeof(path), "%s/comoara.bin", entry->d_name);
            int fd = open(path, O_RDONLY);
            if (fd < 0) continue;

            char buf[168];
            ssize_t r;
            while ((r = read(fd, buf, sizeof(buf))) == sizeof(buf)) {
                count++;
            }

            close(fd);
            snprintf(buffer, sizeof(buffer), "Hunt: %s, Count: %d\n", entry->d_name, count);
            tiparire(buffer);
        }
    }

    closedir(d);
}

void list_treasures(int sig) {
    char buffer[512];
    ssize_t n = read(cmdpipe[0], huntid, sizeof(huntid));
    if (n <= 0) {
        tiparire("Failed to read huntid\n");
        return;
    }
    huntid[strcspn(huntid, "\n")] = 0;

    int pipeout[2];
    pipe(pipeout);
    pid_t child = fork();
    if (child == 0) {
        dup2(pipeout[1], STDOUT_FILENO);
        close(pipeout[0]);
        close(pipeout[1]);
        execlp("./main", "main", "--list", huntid, (char *)NULL);
        perror("execlp");
        _exit(1);
    } else {
        close(pipeout[1]);
        waitpid(child, NULL, 0);
        while ((n = read(pipeout[0], buffer, sizeof(buffer))) > 0) {
            write(pipefd[1], buffer, n);
        }
        close(pipeout[0]);
    }
}

void view_treasure(int sig) {
    char buffer[512];
    char fullbuf[64];
    ssize_t n = read(cmdpipe[0], fullbuf, sizeof(fullbuf));
    if (n <= 0) {
        tiparire("Failed to read huntid/id\n");
        return;
    }

    sscanf(fullbuf, "%19s %19s", huntid, id);

    int pipeout[2];
    pipe(pipeout);

    pid_t child = fork();
    if (child == 0) {
        dup2(pipeout[1], STDOUT_FILENO);
        close(pipeout[0]);
        close(pipeout[1]);
        execlp("./main", "main", "--view", huntid, id, (char *)NULL);
        perror("execlp");
        _exit(1);
    } else {
        close(pipeout[1]);
        waitpid(child, NULL, 0);
        while ((n = read(pipeout[0], buffer, sizeof(buffer))) > 0) {
            write(pipefd[1], buffer, n);
        }
        close(pipeout[0]);
    }
}

void calculate_scores() {
    DIR *d = opendir(".");
    if (!d) {
        perror("opendir");
        return;
    }

    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char path[4096];
            snprintf(path, sizeof(path), "%s/comoara.bin", entry->d_name);

            int fd = open(path, O_RDONLY);
            if (fd < 0) continue;
            close(fd);

            int pipeout[2];
            if (pipe(pipeout) < 0) {
                perror("pipe");
                continue;
            }

            pid_t child = fork();
            if (child == 0) {
                dup2(pipeout[1], STDOUT_FILENO);
                close(pipeout[0]);
                close(pipeout[1]);
                execlp("./score", "score_calculator", entry->d_name, (char *)NULL);
                perror("execlp");
                _exit(1);
            } else if (child > 0) {
                close(pipeout[1]);
                char buffer[512];
                ssize_t n;
                while ((n = read(pipeout[0], buffer, sizeof(buffer))) > 0) {
                    write(STDOUT_FILENO, buffer, n);
                }
                close(pipeout[0]);
                waitpid(child, NULL, 0);
            } else {
                perror("fork");
                continue;
            }
        }
    }

    closedir(d);
}

void stop_monitor(int sig) {
    usleep(500000);
    exit(0);
}

void monitor_procces() {
    close(pipefd[0]);
    close(cmdpipe[1]);

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

    while (1) pause();
}

int main(int argc, char **argv) {
    char m[50];

    if (pipe(pipefd) == -1 || pipe(cmdpipe) == -1) {
        perror("pipe");
        exit(1);
    }

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
                    close(pipefd[1]);
                    close(cmdpipe[0]);
                    run = 1;
                }
            }

        } else if (strcmp(m, "list_hunts") == 0) {
            if (!run) printf("Monitorizarea nu ruleaza\n");
            else {
                kill(p, SIGUSR1);
                usleep(100000);
                char buf[512];
                ssize_t n;
                while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
                    write(STDOUT_FILENO, buf, n);
                    if (n < sizeof(buf)) break;
                }
            }

        } else if (strcmp(m, "list_treasures") == 0) {
            if (!run) printf("Monitorizarea nu ruleaza\n");
            else {
                printf("Enter huntid: ");
                if (scanf("%19s", huntid) != 1) continue;

                write(cmdpipe[1], huntid, strlen(huntid));
                write(cmdpipe[1], "\n", 1);

                kill(p, SIGUSR2);
                usleep(100000);

                char buf[512];
                ssize_t n;
                while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
                    write(STDOUT_FILENO, buf, n);
                    if (n < sizeof(buf)) break;
                }
            }

        } else if (strcmp(m, "view_treasure") == 0) {
            if (!run) printf("Monitorizarea nu ruleaza\n");
            else {
                printf("Enter huntid: ");
                if (scanf("%19s", huntid) != 1) continue;
                printf("Enter id: ");
                if (scanf("%19s", id) != 1) continue;

                char buf[64];
                snprintf(buf, sizeof(buf), "%s %s\n", huntid, id);
                write(cmdpipe[1], buf, strlen(buf));

                kill(p, SIGILL);
                usleep(100000);

                char out[512];
                ssize_t n;
                while ((n = read(pipefd[0], out, sizeof(out))) > 0) {
                    write(STDOUT_FILENO, out, n);
                    if (n < sizeof(out)) break;
                }
            }

        } else if (strcmp(m, "calculate_score") == 0) {
            if (!run) printf("Monitorizarea nu ruleaza\n");
            else {
                calculate_scores();
            }

        } else if (strcmp(m, "stop_monitor") == 0) {
            if (!run) printf("Monitorizarea nu ruleaza\n");
            else {
                kill(p, SIGTERM);
                waitpid(p, NULL, 0);
                run = 0;
                printf("Monitorizarea oprit\n");
            }

        } else if (strcmp(m, "exit") == 0) {
            if (run) printf("Monitorizarea ruleaza\n");
            else exit(0);
        } else {
            printf("Comanda necunoscuta\n");
        }
    }
}
