int main(int argc, char **argv)
{
	for(int i = 0; i < 100; i++) {
		if(!(i % 3)) printf("Fizz ");
		else if(!(i % 5)) printf("Buzz ");
		else if(!(i % 15)) printf("Fizzbuzz ");
		else printf("%d ", i);
	}

	return 0;
}
