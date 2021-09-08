#include <iostream>   
#include <math.h>   
#include <stdlib.h>   
#include <unistd.h>
using namespace std;   
  
int main()   
{   
     const float PI=3.1416;   
     int count=180;     //???????   
     int idle[count];   
     int busy[count];   
     float delta=2*PI/count;   
     float alpha=0;   
     /*
        ????sleep?????????
        busy?????????busy????sleep????
      */  
     for(int i=0;i<count;i++)   
      {   
         busy[i]=count*(sin(alpha)+1)/2;   
         idle[i]=count-busy[i];   
         alpha=alpha+delta;   
         cout<<busy[i]<<"---"<<idle[i]<<endl;        
      }   
        
     int j=0;   
     int st_time;   
     while(true)   
      {      
          j=j%count;   
          st_time=clock();     //????   
         while((clock()-st_time)<busy[j]);   
          sleep(idle[j]);   
          j++;   
      }          
      system("PAUSE");   
     return 0;   
}