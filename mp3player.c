#include <mpg123.h>
#include <ao/ao.h>
#include <termios.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BITS 8
#define WORKDIR "/home/mungkin/Documents/sisop/fp/coba/"

void* display(void *arg);
void* play_music(void *arg);
void changemode(int);
int  kbhit(void);
bool ext_match(const char *name, const char *ext);

int play=0;
int finish=0;
int trans=0;
int song_index=0;
int song_amt=0;
char playlist[1000][1000];

int main(){
    int i=0;
    pthread_t thread;
    char ch;
    changemode(1);

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (WORKDIR)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            struct stat st;
            memset(&st, 0, sizeof(st));
		    st.st_mode = ent->d_type << 12;
            if(!S_ISDIR(st.st_mode) || ext_match(ent->d_name, ".mp3")){
                strcpy(playlist[i++], ent->d_name);
            }
        }
        closedir (dir);
    } else {
        perror ("");
        return EXIT_FAILURE;
    }
    song_amt=i;

    // play_music(NULL);

    if(pthread_create(&thread, NULL, display, NULL)<0){
        perror("could not create thread");
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&thread, NULL, play_music, NULL)<0){
        perror("could not create thread");
        exit(EXIT_FAILURE);
    }


    while(1){
        while(!kbhit());
        ch = getchar();
        if(ch=='k'){
            play=!play;
        }else if(ch=='j'){
            song_index+=song_amt;
            song_index--;
            play=0;
            trans=1;
            finish=1;
        }else if(ch=='l'){
            song_index++;
            play=0;
            trans=1;
            finish=1;
        }else if(ch=='q'){
            changemode(0);
            exit(0);
        }   
    }return 0;
}

void* display(void *arg){
    while(1){
        int i;
        for(i=0; i<song_amt;i++){
            if(i == song_index){
                printf("[*] %s\n", playlist[i]);
            }else{
                printf("[%d] %s\n", i, playlist[i]);
            }
        }
        sleep(1);
        system("clear");
    }
}

void* play_music(void *arg){
    mpg123_handle *mh;
    unsigned char *buffer;
    size_t buffer_size;
    size_t done;
    int err;

    int driver;
    ao_device *dev;

    ao_sample_format format;
    int channels, encoding;
    long rate;

    /* initializations */
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    while(1){
        char file[1000];
        strcpy(file, WORKDIR);
        strcat(file, playlist[song_index]);

        /* open the file and get the decoding format */
        mpg123_open(mh, file);
        mpg123_getformat(mh, &rate, &channels, &encoding);

        /* set the output format and open the output device */
        format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
        format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = 0;
        dev = ao_open_live(driver, &format, NULL);

        if(trans){
            play=1;
            trans=0;
        }

        finish=0;

        /* decode and play */
        while(!finish){
            printf(" \b\b\b\b\b");
            if(finish){
                break;
            }
            if(play){
                // printf("waks");
                if(mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK){
                    ao_play(dev, buffer, done);
                }else{
                    finish=1;
                }
            }else{
                continue;
            }
        }

        if(play && trans){
            song_index+=1;
        }
        song_index%=song_amt;
    }

    /* clean up */
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
    // play_music(NULL);
}

void changemode(int dir){
  static struct termios oldt, newt;
 
  if ( dir == 1 )
  {
    tcgetattr( STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
  }
  else
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
}
 
int kbhit (void){
  struct timeval tv;
  fd_set rdfs;
 
  tv.tv_sec = 0;
  tv.tv_usec = 0;
 
  FD_ZERO(&rdfs);
  FD_SET (STDIN_FILENO, &rdfs);
 
  select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &rdfs);
}

bool ext_match(const char *name, const char *ext){
	size_t nl = strlen(name), el = strlen(ext);
	return nl >= el && !strcmp(name + nl - el, ext);
}
