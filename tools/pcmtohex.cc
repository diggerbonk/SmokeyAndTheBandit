#include <stdio.h>
#include <string.h>

//PCM2Hex - Lee Weber(D3thAdd3r) 2010
//Released under GPL 3.0 or later.

//Converts a RAW 8 bit mono PCM file (no headers) to  formatted hex nicely in a PROGMEM array.
//Cool Edit and many other programs allow you to save in this format. The sample rate will depend
//on the speed you will play it back on the Uzebox via TriggerNote(). See the tutorial if you have
//questions.


int main(int argc, char *argv[])
{
   char *varname = "PCM_Data";
   char *inname = "in.pcm";
   
   if(argc < 2){
      printf( "\n\tUsage: input.raw outfile[optional]\n\n"          \
                "\t-v  [specify custom variable name]\n\n"          \
                "\tRAW PCM 8 bit unsigned mono files supported.\n"  \
                "\tEx:  pcm2hex input.raw out.inc -v VarName\n\n");
      return 0;
   }
   else
      inname = argv[1];
   
   for(int i=0;i<argc;i++){

      if(!strcmp(argv[i],"-v")){//variable name
         if(argv[i+1]){
            varname = argv[i+1];
            i++;
         }
         else
            printf("\n\tBad variable name, using default.\n");     
      }        
   }
   
   FILE * fin = fopen(inname,"rb");
   FILE * fout;
   
   if(argc > 2)//user has supplied output file name
      fout = fopen(argv[2],"w");
   else
      fout = fopen("PCM_out.inc","w");
   
   if(fin == NULL){
      printf("Failed to open file.\n");
      fcloseall();
      return 0;     
   }
   
   if(fout == NULL){
      printf("Failed to create file.\n");
      fcloseall();
      return 0;
   }
   
   int wide = 0;
   unsigned int numbytes = 0;
   unsigned char c;
   int b;
   
   fprintf(fout,"const char %s[] PROGMEM = {\n",varname);
   
   while(!feof(fin)){
      fscanf(fin,"%c",&b);
      fprintf(fout,"0x");
      
      b += 128;//convert to unsigned
      c = b;
      
      if(c < 16)//don't know how to handle this gracefully. Width parameter with escape characters? Never needed them before...
         fprintf(fout,"0");
         
      fprintf(fout,"%X",c);//output hex format
      if(!feof(fin))
         fprintf(fout,",");//skip last comma
      else if(wide < 15)
         fprintf(fout," ");//use space if not too wide, otherwise closing bracket is on next line.
         
      if(++wide > 15){//16 values per line
         wide = 0;
         fprintf(fout,"\n");
      }
      numbytes++;
   }
   fprintf(fout,"};\n");
   fcloseall();
   
   printf("\n\tDone. PROGMEM usage: %d bytes\n",numbytes);
}
