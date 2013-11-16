
#include "serial.h"
#include <memory.h>
#include <fcntl.h>
#include <cstdio>
//#include <sys/select.h>

SerialPort::SerialPort() {
  m_fd = -1;
}

SerialPort::~SerialPort() {
  closePort();
}

bool SerialPort::openPort(const char* device, int baudrate) {
  printf("open()\n");
  m_fd = open(device, O_RDWR | O_NOCTTY);
  if(m_fd <= 0){
    return false;
  }

  //�^�C���A�E�g�����p
  m_timeout.tv_usec = 0;
  m_timeout.tv_sec = 1;
  FD_ZERO(&m_readfs);
  FD_SET(m_fd, &m_readfs);

  printf("copy oldsettings\n");
  tcgetattr(m_fd, &m_oldTio); //�ޔ�

  struct termios tio;
  memset(&tio, 0, sizeof(tio));
  tio.c_cflag = CS8 | CLOCAL | CREAD;
  tio.c_iflag=IGNPAR;          /* IGNPAR:�p���e�B�G���[�̕����͖��� */

  tio.c_oflag=0;               /* raw���[�h */
  tio.c_lflag=0;               /* ��J�m�j�J������ */

//  tio.c_cc[VTIME]=0;           /* �L�����N�^�ԃ^�C�}�͖��g�p */
  tio.c_cc[VTIME]=3;           /* �L�����N�^�ԃ^�C�}�͖��g�p */
  tio.c_cc[VMIN]=1;            /* 1�����󂯎��܂Ńu���b�N���� */

  cfsetispeed(&tio, baudrate);
  cfsetospeed(&tio, baudrate);

  printf("flush\n");
  tcflush(m_fd,TCIFLUSH);           /* �|�[�g�̃N���A */
  printf("apply settings\n");
  tcsetattr(m_fd, TCSANOW, &tio); /* �|�[�g�̐ݒ��L���ɂ��� */
  printf("done\n");
  return true;
}

void SerialPort::closePort(){
  if(m_fd >= 0){
    tcsetattr(m_fd, TCSANOW, &m_oldTio);
    close(m_fd);
  }
}

int SerialPort::putChar(unsigned char c) {
  return write(m_fd,&c,1);
}

int SerialPort::putBytes(const unsigned char* bytes, int num) {
  return write(m_fd, bytes, num);
}

bool SerialPort::getChar(unsigned char* c) {
  int ret = select(m_fd + 1, &m_readfs, NULL, NULL, &m_timeout);
  if(ret > 0){
    read(m_fd, c, 1);
    return true;
  }
  else if(ret == 0){
    printf("timeout\n");
    return false;
  }
  else{
    printf("error\n");
    return false;
  }
}

int SerialPort::getBytes(unsigned char* bytes, int num) {
  return read(m_fd, bytes, num);
}
