/*    CSE3320 - HW1
     Name: Ghaida Al-Atoum 
 */
#include <ncurses.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <dirent.h> 
#include <cstring>
#include <time.h>
#include <vector>
#include <errno.h>
#include <algorithm> 
#include<iostream>
#include <sys/stat.h>
using namespace std;
struct list 
{
    string name;
	int size;
	time_t t;
	list(string n,int s,time_t ti)
	{
		name = n;
		size =s; 
		t = ti;
	}
};
bool comparesize(const list &a, const list &b){
	return a.size < b.size ;
}
bool compareDate(const list &a, const list &b){
	double diff = difftime(a.t,b.t);
	if( diff >0) return 0; // a>b
	else return 1;  // a<=b
}
  DIR *d; 
  struct dirent * de;
  vector<list> Direc;
  vector<list> Files;
  int numD=0,numF=0;
  WINDOW * Fwin;
  WINDOW * Dwin;
  WINDOW * Opp;
void checkForErrno(int st){
	if( errno == EACCES) 
	{mvprintw(st,1," Permission to read was denied \n"); refresh();}
	if( errno == ERANGE )
	{mvprintw(st,1," Size Argument is less than the length \n ");refresh();}
	if( errno == ENFILE )
	{	mvprintw(st,1," The process has too many files open.  \n ");refresh();}
	if( errno == ENOMEM )
	{	mvprintw(st,1," Not enough memory available.   \n ");refresh();}
}
void Read(){
	Direc.clear(); Files.clear(); struct list temp(" ",0,0);
	struct stat buf;
	// count, and save directories and files
  d = opendir( "." );
  checkForErrno(1);
  while( (de = readdir(d) ) ) 
  {   stat(de->d_name,&buf); temp.size= buf.st_size; temp.t=buf.st_atime;
 	if ( (de->d_type) & DT_DIR )
	{   temp.name = de->d_name; 
		Direc.push_back(temp);
	}
    if ( (de->d_type) & DT_REG )
	{   temp.name = de->d_name;
	 	Files.push_back(temp);
	}
  }
  closedir( d );
}
void printDirec(int start,int num){  
	for( int i =1; i <num && start < numD;i++)
	{
		mvwprintw(Dwin,i,1,"%d. %s\t(%d),%s",start,(Direc.at(start).name).c_str(),(Direc.at(start).size),ctime(&(Direc.at(start).t)));
		start++;
	}
}
void printFiles(int start,int num){      		
    for( int i =1; i<num && start < numF; i++)
	{   
		mvwprintw(Fwin,i,1,"%d. %s (%d),%s",start,(Files.at(start).name).c_str(),(Files.at(start).size),ctime(&(Files.at(start).t)));
	    start++;
	}
}
void refreshes(){
	  refresh();
  wrefresh(Dwin);
  wrefresh(Fwin); 
  wrefresh(Opp);
}
void SortFun(char cmd){
  switch(cmd)
  {
	  case '1': // Size
		  for(auto &&i: Direc)
		  {
			std::sort(Direc.begin(),Direc.end(),comparesize);
		  }
		  for(auto &&i: Files)
		  {
            std::sort(Files.begin(),Files.end(),comparesize);
		  }
		  break;
	  case '2': //Date
		  for(auto &&i: Direc)
		  {
			std::sort(Direc.begin(),Direc.end(),compareDate);
		  }
		  for(auto &&i: Files)
		  {
            std::sort(Files.begin(),Files.end(),compareDate);
		  }
		  break;
  }
}
int main(int argc,char ** argv){
  initscr();
  refresh();
  cbreak();
  string pico;
  time_t t;
  pid_t pid;
  string choices[9] = {"nextF","prevF","nextD","prevD","Edit","Run","Chdir","Sort","quit"};
  int choice, highlight =0;
  char *ptr;
  char *str[3];	string gg; 
  char s[256];
  char cmd,cmd2,cmd3;
  size_t size = 200;
  int iD=0,iF=0,num;
  int activeNF=0,activeND=0,chdirindicator=0;
  if( argc == 2 )
	{
		if( chdir(argv[1]) != 0)
		{printw(" Failed to Open %s press any key to continue",argv[1]);refresh();getchar();
        checkForErrno(1);}
	}
  Read();
	clear();
  while(1){
	  
  // count, and save directories and files
	numD = Direc.size(); numF= Files.size();
  // Dimensions of the Screen 
  int ymax=0,xmax=0;
	  getmaxyx(stdscr,ymax,xmax);

  while(ymax < 20)
  {
  clear();
  mvprintw(0,0," Increse Window size ");
  getmaxyx(stdscr,ymax,xmax);
  refresh();
  }
  clear();	
  // Windows Dimensions Dimensions
  int Dlines,Flines,Dcol,Fcol,temp,start=0;
  temp = ymax - 10;
  Dlines = Flines = (temp /3) ; // 8 Names displayed a time
  if( Dlines < numD) activeND =1; else activeND=0;
  if( Flines < numF) activeNF =1; else activeNF=0;
  Dcol = Fcol = xmax-1; refreshes();
  // Time
  t = time(NULL);
  mvprintw(start,0,"\nTime: %s\n", ctime(&t)); start+=2; refreshes();
  // Current Working Directory
  ptr = getcwd(s,size);
  mvprintw(start,0,"Current Directory: %s ",s); start += 2;
  // DIRECTORIES 
  mvprintw(start,0,"Directories: "); start +=1;
  Dwin = newwin(Dlines,Dcol,start,0);  refreshes();
  box(Dwin,0,0);
  start += Dlines;
  // FILES
  mvprintw(start,0,"Files ");
  start += 1;
  Fwin = newwin(Flines,Fcol,start,0);  refreshes();
  box(Fwin,0,0);
  start += Flines;
  Flines --; Dlines--;
  // PRINT DIREC AND FILES
  printDirec(iD,Dlines);
  printFiles(iF,Flines);
  // COMMANDS WINDOW
  mvprintw(start,0,"Opperations"); start+=1;
  Opp = newwin(5,Fcol,start,0); start +=5;
  box(Opp,0,0);
	 int k2=1, k3=1;
  for( int k=0;k<9;k++){
	  if( k == 4) { k3=2; k2=1; }
	mvwprintw(Opp,k3,k2,"%d) %s",k,choices[k].c_str());
    k2+=8;
   }
	  k3++;
	mvwprintw(Opp,k3,1," Enter the Opperation number 0-8 ");
    refreshes();
  // INPUT 
	refreshes();
    cmd= getchar();
	switch(cmd) {
		case '0': // NEXT FILES
			  if( activeNF ) 
		      { 
				iF +=( Flines-1);
				if(iF >= numF) iF -= (Flines-1);
			    printFiles(iF,Flines);
					refreshes();
			  }
			  break;
		case '1': // PREV FILES
			  if( activeNF ) 
		      { 
				 if(iF != 0) 
			  {
				iF -= (Flines-1);
			    printFiles(iF,Flines); 	refreshes();
			  }
			  }
			  break;
		case '2':// NEXT DIRECTORIES
			if( activeND ) 
		      { iD += (Dlines-1);
				if(iD >= numD) iD -= (Dlines-1);				
			    printDirec(iD,Dlines); 	refreshes();
			  }
			  break;
		case '3': //PREV DIRECTORIES
			if( activeND ) 
		      { if(iD != 0) 
			  {
				iD -= Dlines-1;
			     printDirec(iD,Dlines);	refreshes();
			  }
			  }
			  break;
		case '4': // EDIT
			  memset(s,' ',256);   pico = "pico";
			  mvprintw(start,1,"Edit What : ");	refreshes(); 
			  getstr(s); gg=s;  
			  str[1]=(char*)gg.c_str();
			  str[0]=(char*)pico.c_str();
			  str[2]=NULL;
              pid=fork();
		      if(pid ==0 )
	          { 
		        if( execvp (str[0],str) == -1)
				{ mvprintw(start+3,1,"ERROR"); refresh();usleep(10000000);refreshes(); }
	          }
              if(pid > 0)
	          {  
		        wait(0); 	 clear();refreshes();
			  }
			  break;
		case '5': //RUN
			  memset(s,' ',256);
			  mvprintw(start,1,"Run What : ");	 refreshes(); 
			  getstr(s); gg=s; 
			  str[0]=(char*)gg.c_str();
			  str[1]=NULL;
              pid=fork();
		      if(pid ==0 )
	          { 
		        if( execvp (str[0],str) == -1)
				{   mvprintw(start+3,1,"ERROR "); refresh();usleep(1000000);refreshes();}
	          }
              if(pid > 0)
	          {  
		        wait(0);
				  // For curses library 
				  clear();	refreshes();
			  }
			  break;
		case '6': // Change DIR
			  memset(s,' ',256);
			  mvprintw(start,1,"Change to What : ");	 refreshes(); 
			  getstr(s); gg=s; str[0]=(char*)gg.c_str();  chdir(str[0]); Read();
			  refreshes();
			  break;
		case '7': // Sort
			  mvprintw(start,1,"Sort By Size enter (1) By Date enter (2) : ");refreshes();  clear();
			  cmd2 = getchar(); SortFun(cmd2);
			break;
		case '8': // Quit
			  clear();
			  endwin();
			  exit(0);
			  break;
	}
	}
	endwin();
  return 0;
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	