#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

typedef struct {
    char tresureid[40];
    char name[40];
    float x;
    float y;
    char text[100];
    int val;
}tr;

void userlog(const char *hunt,const char *mesaj) {

    char path[100];
    strcpy(path,hunt);
    strcat(path,"/userlog");

    int f=open(path,O_WRONLY|O_CREAT|O_APPEND,0644);
    if (f == -1) {
        perror("Error opening file");
        printf("Error");
    }


    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "[%Y-%m-%d %H:%M:%S]", t);


   char m[256];
    strcpy(m,timebuf);
    strcat(m,mesaj);
    write(f, &m, strlen(m));

    close(f);

}
void add(const char *hunt)
{
    tr t;
    struct stat st;
    if (stat(hunt, &st) != 0) {
        if (mkdir(hunt,0777)==-1) {
            perror("mkdir");
            exit(1);
        }
        char buff[256];
        sprintf(buff, "utilizatorul a creat %s\n",hunt);
        userlog(hunt,buff);
    }

   char buff[256];

    printf("Treasure ID: ");
    scanf("%39s",t.tresureid);
    buff[0] = '\0';
    sprintf(buff, "utilizatorul a introdus tresure id: %s\n",t.tresureid);
    userlog(hunt,buff);



    printf("nume:");
    scanf("%39s",t.name);
    buff[0] = '\0';
    sprintf(buff, "utilizatorul a introdus numele: %s\n",t.name);
    userlog(hunt,buff);
    int k=0;

    while(k==0) {
        printf("latitudine: ");
        if(scanf("%f",&t.x)<=0)
        {
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus gresit latitudinea\n");
            userlog(hunt,buff);
            printf("Incorrect input\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        } else {
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus latitudinea: %f\n",t.x);
            userlog(hunt,buff);
            k = 1;
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
    }

    k=0;

    while (k==0) {
        printf("longitudine: ");
        char buffer[100];
        fgets(buffer,100,stdin);
        char *endptr;
        if (strtof(buffer,&endptr)==0) {
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus gresit longitudinea\n");
            userlog(hunt,buff);
            printf("Incorrect input\n");
        }
        else {
            k=1;
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus latitudinea: %f\n",t.y);
            userlog(hunt,buff);
        }
    }

    printf("text:");
    fgets(t.text, sizeof(t.text), stdin);
    buff[0] = '\0';
    sprintf(buff, "utilizatorul a introdus textul: %s\n",t.text);
    userlog(hunt,buff);


    k=0;

    while(k==0) {
        printf("val:");
        if(scanf("%d",&t.val)<=0) {
            printf("Incorrect input\n");
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus gresit longitudinea\n");
            userlog(hunt,buff);
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        } else {
            k = 1;
            buff[0] = '\0';
            sprintf(buff, "utilizatorul a introdus valoarea: %d\n",t.val);
            userlog(hunt,buff);



        }
    }

    char path[100];
    strcpy(path,hunt);
    strcat(path,"/comoara.bin");

    int f=open(path,O_WRONLY|O_CREAT|O_APPEND,0644);
    if (f == -1) {
        perror("Error opening file");
        printf("Error");
    }
    ssize_t bytes_written = write(f, &t, sizeof(tr));
    if (bytes_written != sizeof(tr)) {
        perror("write");
        close(f);

    }
    else {
        perror("write");
        buff[0] = '\0';
        sprintf(buff, "utilizatorul a introdus o comoara noua\n");
        userlog(hunt,buff);
    }

    char path1[100];
    strcpy(path1,hunt);
    strcat(path1,"/userlog");

    char path2[100];
    strcpy(path2,path1);
    strcat(path2,"-");
    strcat(path2,hunt);
    if (symlink(path1,path2) == -1) {
        perror("symlink");
    }
    close(f);



}

void list(const char *hunt)
{
    printf("hunt name: %s",hunt);

    char path[100];
    strcpy(path,hunt);
    strcat(path,"/comoara.bin");

    int f=open(path,O_RDONLY);
    if (f == -1) {
        perror("Error opening file");
        printf("Error");
    }
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("Error getting file stats");
        close(f);
        return;
    }
    off_t file_size = st.st_size;
    time_t last_modified = st.st_mtime;


    char time_str[100];
    struct tm *time_info = localtime(&last_modified);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    printf("\nFile size: %ld bytes\n", file_size);
    printf("Last modified: %s\n", time_str);

    tr t;
    ssize_t bytes_read;
    int k = 0;
    while ((bytes_read = read(f, &t, sizeof(tr))) > 0) {
        if (bytes_read != sizeof(tr)) {
            printf("Error reading record\n");
            break;
        }

        printf("Comoara %d:\n", ++k);
        printf("  ID: %s\n", t.tresureid);

    }

    if (bytes_read == -1) {
        perror("Error reading file");
    }
    close(f);
    userlog(hunt,"utilizatorul a afisat comorile din hunt\n");



}
void view(const char *hunt,const char *game)
{

    char path[100];
    strcpy(path,hunt);
    strcat(path,"/comoara.bin");

    int f=open(path,O_RDONLY);
    if (f == -1) {
        perror("Error opening file");
        printf("Error");
    }

    tr t;
    ssize_t bytes_read;
    int k=0;
    while ((bytes_read = read(f, &t, sizeof(tr))) > 0) {
        if (bytes_read != sizeof(tr)) {
            printf("Error reading record\n");
            break;
        }


       if (strcmp(game,t.tresureid)==0) {
           printf("  ID: %s\n", t.tresureid);
           printf("  nume: %s\n", t.name);
           printf("  latitudine: %f\n", t.x);
           printf("  longitudine: %f\n", t.y);
           printf("  text: %s\n", t.text);
           printf("  val: %d\n", t.val);
           k=1;
           break;

       }
        if (bytes_read == -1) {
            perror("Error reading file");
        }

    }
    if (k==0) {
        printf("nu a fost gasit");
    }

    close(f);
    userlog(hunt,"vizualizare informatii despre comoara\n");
}
void remove_t(const char *hunt,const char *game)
{
    char path[100];
    strcpy(path,hunt);
    strcat(path,"/comoara.bin");

    int f=open(path,O_RDONLY);
    if (f == -1) {
        perror("Error opening file");
        printf("Error");
    }
    char buffer[100];
    strcpy(buffer,hunt);
    strcat(buffer,"/temporar.bin");
    int b=open(buffer,O_WRONLY|O_CREAT|O_TRUNC,0644);
    if (b == -1) {
        perror("Error opening file");
        printf("Error");
    }

    tr t;
    int removed = 0;

    while (read(f, &t, sizeof(tr)) == sizeof(tr)) {
        if (strcmp(t.tresureid, game) == 0) {
            removed = 1;
            continue;
        }
        write(b, &t, sizeof(tr));
    }

    close(b);
    close(f);

    if (removed) {

        if (remove(path) != 0 || rename(buffer, path) != 0) {
            perror("Error replacing original file");
        } else {
            printf("Treasure with ID '%s' was removed.\n", t.tresureid);
        }
    } else {
        remove(buffer);
        printf("Treasure with ID '%s' not found.\n", t.tresureid);
    }

    char buff[256]="";
    sprintf(buff, "utilizatorul %s a sters \n",game);
    userlog(hunt,buff);

}
void remove_h(const char *hunt )
{
    char path[100];
    strcpy(path,hunt);
    strcat(path,"/comoara.bin");
    if (remove(path) != 0 )
        perror("Eroare stergere comoara.bin");
    else
        printf("comoara .bin sters\n");

    path[0] = '\0';
    strcpy(path,hunt);
    strcat(path,"/userlog");

    if (remove(path) != 0 )
        perror("Eroare stergere userlog");
    else
        printf("userlog sters\n");
    strcat(path,"-");
   strcat(path,hunt);

    if (remove(path) != 0 )
        perror("Eroare stergere symlink");
    else
        printf("symlink sters\n");
    if (rmdir(hunt) != 0) {
        perror("eroare");
    } else {
        printf("Hunt '%s' a fost sters.\n", hunt);
    }

}
int main(int argc,char* argv[]) {

    if(strcmp(argv[1],"--add")==0)
    {
        add(argv[2]);
    }
    else
        if(strcmp(argv[1],"--list")==0)
        {
            list(argv[2]);
        }
        else
            if(strcmp(argv[1],"--view")==0)
            {
                view(argv[2],argv[3]);
            }
            else
                if(strcmp(argv[1],"--remove_t")==0)
                {
                    remove_t(argv[2],argv[3]);
                }
                else
                    if(strcmp(argv[1],"--remove_h")==0)
                    {
                        remove_h(argv[2]);
                    }


    return 0;
}

