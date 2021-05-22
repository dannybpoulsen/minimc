void assert (int);

int main () {
  int i = 0;
  int m = 0;
  while (1) {
	switch (i) {
	case 0:
	  i = 1;
	  break;
	case 1:
	  i = 2;
	  break;
	case 2:
	  i = 3;
	default:
	  goto end;
	}
  }
 end:
  
  m = i;
  int n = m+1;
  assert(n == 4);
}
