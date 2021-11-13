#include <cstdio>
#include <string>
#include <stdexcept>
#include <algorithm>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

int open_port(const char * port = "/dev/ttyUSB0")
{
  int fd = -1;

  fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd == -1)
    throw std::runtime_error("open failed");
  else
    fcntl(fd, F_SETFL, 0);

  struct termios attr;
  if (tcgetattr (fd, &attr) != 0)
    throw std::runtime_error("tcgetattr failed");

  cfsetospeed(&attr, B57600);
  cfsetispeed(&attr, B57600);

  if (tcsetattr (fd, TCSANOW, &attr) != 0)
    throw std::runtime_error("tcsetattr failed");

  return fd;
}

int main()
{
  char buffer[2048];
  int fd = -1;

  try {
    fd = open_port();
    size_t num_chars = 0;
    while (true) {
      num_chars = read(fd, buffer, sizeof(buffer));
      if (num_chars != -1 && num_chars > 0) {
        printf("%d: ", num_chars);
        for (size_t i=0; i < num_chars; i++)
          printf("%02x", buffer[i]);
        printf("\n");
      }
      usleep(10000);
    }
  } catch (std::exception &ex) {
    printf("Caught exception: %s\n", ex.what());
  } catch (...) {
    printf("Caught unknown exception\n");
  }

  if (fd != -1)
    close(fd);

  return 0;
}