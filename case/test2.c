int main()
{
 int a,b,f;
 float fnum = 3.2;
  
 a = read();
 b = read();
 f = a*b;

 write("fnum ");
 write(fnum);
 write("\n");
 write("fnum < a ");
 write((fnum < a));
 write("\n");
 write("!(fnum < a) ");
 write(!(fnum < a));
 write("\n");
 write("b/a ");
 write(b/a);
 write("\n");
 write("b/a<0 ");
 write(b/a<0);
 write("\n");
 write("f ");
 write(f);
 write("\n");
 write("!f ");
 write(!f);
 write("\n");
 if( !(fnum < a) || b/a  < 0 && !f)
 {
	write( "True \n");
 }
 else
 {
	write("Fales \n");
 }   
   
 return 1;
}


