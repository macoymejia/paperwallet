#include <panic.h>
#include <progress.h>
#include <bignat.h>
#include <bigint.h>
#include <sha256.h>
#include <rmd160.h>
#include <base58.h>
#include <bitelliptic.h>
#include <bitaddress.h>
#include <rng.h>
#include <qrencode.h>
#include <m190printer.h>

const int LEDPIN=8;
const int BUTTONPIN=11;

void PANIC(byte errorCode){
  for(;;){
    for(byte i=0;i<errorCode;i++){
      digitalWrite(LEDPIN,HIGH);
      delay(250);
      digitalWrite(LEDPIN,LOW);
      delay(250);
    }
    delay(1000);
  }
}

void PROGRESS(float progress){
  static byte flipflop=0;
  flipflop++;
  digitalWrite(LEDPIN,flipflop%2);
}

uint8_t PRIVATE[32];
WORD xbuf[32];
WORD ybuf[32];
struct bigint pubx(xbuf,sizeof(xbuf));
struct bigint puby(ybuf,sizeof(ybuf));

const float qrxscale=2.25;
const int qryscale=2;

//const float qrxscale=1.125;
//const int qryscale=1;

//const int qryscale=3;
//const float qrxscale=3.375;

boolean qrcodesource(void *ctx,int x,int y){
  qrcontext *qr=(qrcontext *)ctx;

  float fx = (float)x/qrxscale;
  x=fx;
  y/=qryscale;
  if(x<0||x>WD||y<0||y>WD){
    return 0;
  } else {
    return qr->getQRBit(x,y);
  }
}

byte qroffset=20;

boolean qrcodesource2(void *ctx,int x,int y){
//  return( x==10 || y==10);
  qrcontext *qr=(qrcontext *)ctx;

  float fx = (float)x/qrxscale;
  x=fx;
  x-=qroffset;
  y/=qryscale;
  if(x<0||x>WD||y<0||y>WD){
    return 0;
  } else {
    return qr->getQRBit(x,y);
  }
}


boolean horizlinesource(void *ctx,int x,int y){
  return 1;  
}

void horizline(){
  m190::print(horizlinesource,NULL,5,true);
  m190::formfeed(2);
}

void printqr(char *encoded){
  qrcontext qr;
  qr.qrencode(encoded);
  m190::print(qrcodesource2,&qr,WD*qryscale,true);
}

void printprivate(uint8_t *PRIVATE){
  char encoded[52];
  bitaddress::generatePrivateWIF(PRIVATE,encoded,sizeof(encoded));
  printqr(encoded);
  m190::formfeed(5);
  m190::print(encoded);
}

void printpublic(const struct bigint &x,const struct bigint &y){
  char encoded[40];
  bitaddress::generateAddress(x,y,encoded,sizeof(encoded)); 
  
  m190::print(encoded);
  m190::formfeed(5);
  printqr(encoded);
}

void generatePrivateKey(uint8_t *PRIVATE){
  rng::generate(PRIVATE);
}

void setup(){
  m190::initialize();  

  pinMode(BUTTONPIN,INPUT);
  pinMode(LEDPIN,OUTPUT);
}


void loop(){  
  digitalWrite(LEDPIN,HIGH);  
  while(digitalRead(BUTTONPIN)==LOW){
  }
  
  m190::formfeed(30);

  generatePrivateKey(PRIVATE);
  
  qroffset=20;
  printprivate(PRIVATE);
  m190::formfeed(75);
  
  bitaddress::generatePublicKey(PRIVATE,pubx,puby);
  
  qroffset=5;
  printpublic(pubx,puby);
  m190::formfeed(20);
}
