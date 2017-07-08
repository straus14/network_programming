#include <stdio.h>
#include <string.h>

int main() {
	double taxrate=7.3, discountrate;
	char buyer[100],seller[100];
    int y;

	double *tmpPtr;

	tmpPtr = &taxrate;

	printf (" value of taxrate: %lf", *tmpPtr);

	discountrate = *tmpPtr;

	printf ("\n value of discountrate: %lf", *tmpPtr);

	printf ("\n address of taxrate: %d", &taxrate);

	printf ("\n address of taxrate (by pointer): %d", tmpPtr);

	printf ("\nenter the name of the buyer: \n");
	scanf ("%s" , buyer);

	strcpy (seller, buyer);
    y = strcmp (seller, buyer);

    if ( y == 0)
    	printf (" strings are same\n");
    else
    	printf (" strings are different\n");

    printf ("\nenter the name of the seller: %s \n", seller);

    strcat ( buyer, seller);

    y = strlen (buyer);
    printf ("%d", y);

return 0;

}

