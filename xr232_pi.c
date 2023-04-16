#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#define BLOCKLEN 256
#define BAUDRATE B57600

int sendChar(int port)
{
  char sende_buffer[BLOCKLEN];
  int x;
  int bytes;

  for(x=0; x<=BLOCKLEN-1; x++)
  {
    sende_buffer[x] = 'U';
  }

  bytes = write (port, sende_buffer, BLOCKLEN);
  if (bytes != BLOCKLEN)
  {
    return 1;
  }
  return 0;
}

void xr232INIT(int fd)
{
  struct termios oldtio, newtio;
  tcgetattr(fd,&oldtio); /* save current serial port settings */
  bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
  /*
          BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
          CRTSCTS : output hardware flow control (only used if the cable has
                    all necessary lines. See sect. 7 of Serial-HOWTO)
          CS8     : 8n1 (8bit,no parity,1 stopbit)
          CLOCAL  : local connection, no modem contol
          CREAD   : enable receiving characters
        */
         newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

        /*
         Raw output.
        */
         newtio.c_oflag = 0;
             tcsetattr(fd, TCSANOW, &newtio);

  sleep(3);
}

void xr232REINIT(int port)
{
  int i = 0;
  i |= TIOCM_RTS; // Pin RTS will be deactivated (-12V)
  i |= TIOCM_DTR; // Pin DTR will be deactivated (-12V)
  ioctl (port, TIOCMBIC, &i);
}

int main(int argc, char **argv)
{
  int bytes;
  char buffer[BLOCKLEN];
  int x;
  unsigned long y=0;

  int port = open ("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
  if (port == -1)
  {
    printf("ERROR!, no device found\n");
    return 1;
  }
  printf("Device found!\n");

  FILE *zfz_datei = fopen("raw_entropy_file.entr", "w+b");
  if(zfz_datei == NULL)
  {
      printf("ERROR! File not created!\n");
      return 1;
  }

  xr232INIT(port);
  printf("Starting creating of entropy!\n");
  while(1)
  {
  sendChar(port);
  //sleep(1);
  usleep(2000);
  bytes = read (port, buffer, BLOCKLEN);
  y=y+bytes;

  if(y<=100)
    printf("bytes = %i ; y = %i bytes\n", bytes, y);
  else if(y>=1000)
    printf("bytes = %i ; y = %i kb\n", bytes, y/1000);
  else if(y>=1000000)
    printf("bytes = %i ; y = %i mb\n", bytes, y/1000000);

  fwrite(&buffer, bytes, 1, zfz_datei);
  if(y >= 1024*1024*1024*1024)
  break;
  }
  printf("...");
  xr232REINIT(port);

  close(port);
  close(zfz_datei);
  printf("Done\n");
}
