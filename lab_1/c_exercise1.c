#include <stdio.h>


int main () 
{
	struct route{ 
   int routeID;
   char descrp[25];
   };
   
   typedef struct route Route;
   
   Route route1, longroutes[10], *routePtr;
   
   printf ("Enter the routeID: ");
   scanf ("%d", route1.routeID);
   printf ("Enter the description: ");
   scanf ("%s", route1.descrp);

   longroutes[3] = route1;
   
   routePtr = longroutes;
   
   routePtr += 3;
   
   printf (" members of element3 of longroute: \n") ;
   printf (" %d \n", routePtr->routeID);
   printf (" %s \n", routePtr->descrp);
   
   return 0;
}
