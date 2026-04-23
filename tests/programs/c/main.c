void inc (int* k) {
  *k = *k+1;
}

int main () {
  int k =  0;
  while (k<=10) {
	inc(&k);
  }
}
