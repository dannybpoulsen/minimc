void assert(int);

void setVal (unsigned int* val) {
  *val = 5;
}

int main () {
  unsigned int arr[10];
  setVal (&arr[5]);
  assert(arr[5] == 5);
}
