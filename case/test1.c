int main()
{
 int a,b,c,d,e,f,g;
  
 a = read();
 b = read();
 c = a + 2; 
 d = b + 4;
 e = a + 1;
 f = b;
 g = c*d - 1; 
 
 write(a);
 write("\n");
 write(b);
 write("\n");
 write(c);
 write("\n");
 write(d);
 write("\n");
 write(e);
 write("\n");
 write(f);
 write("\n");
 write(g);
 write("\n");
 write(a||b||c);
 write("\n");
 write(c && d && (e>1));
 write("\n");
 write((f>1)&&(g<1));
 write("\n");
 
 if (a || b || c && d && (e>1) || (f>1)&&(g<1) )
  { 
    write("True \n");
  }
 else
  {
    write("False \n");
  }

   
 return 1;
}
