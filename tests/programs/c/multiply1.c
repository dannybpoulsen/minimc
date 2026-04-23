
void assert(int);

int mult() {
  int a =5;
  int b = 3;
  int c = a * b;
  return c;
}

int main() {
  int res = mult();
  if (res < 0) {
	goto Error;
  }
  //printf("%d\n",res);
  return 0; 
  
 Error: //printf("ERROR %i", i);
  assert(0);
  return -1;
}
