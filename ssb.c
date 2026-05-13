#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>


// Коды клавиш

#define KEY_F1    0
#define KEY_F2    1
#define KEY_F3    2
#define KEY_F4    3
#define KEY_LEFT  8
#define KEY_TAB   9
#define KEY_ENTER 13
#define KEY_ESC   27
#define KEY_RIGHT 0x18
#define KEY_UP    0x19
#define KEY_DOWN  0x1A
#define KEY_SPACE 0x20
#define KEY_BKSPC 0x7F
#define KEY_R	  0x52
#define KEY_N	  0x4E	// next level

// Ардеса микросхем

static uint8_t *VG75 = 0xC000;    // dummy ptr to rcv buf location
static uint8_t *VT57 = 0xE000;    // dummy ptr to rcv buf location

uchar* radio86rkVideoMem = (uchar*)(0xE1D0 + 78*3 + 8);
uchar  radio86rkVideoBpl = 78;
uchar* vidmem_y2;

void RADIO86RK_SCREEN_END(uint16_t MEM_ADDR, uint8_t FULL_HEIGHT, uint8_t FONT, uint16_t MEM_SIZE, uint8_t HIDDEN_ATTRIB, uint8_t CHAR_GEN) 
{
  VG75[1] = 0; 
  VG75[0] = radio86rkVideoBpl-1; //78-1; 
  VG75[0] = (((FONT&0xF) >= 9) ? 0 : 0x40) | (FULL_HEIGHT-1); 
  VG75[0] = FONT; 
  VG75[0] = ((HIDDEN_ATTRIB) ? 0 : 0x40) | ((FONT&0xF)==9 ? 0x80 : 0) | 0x13; 
  VG75[1]= 0x80;
  VG75[0]= 100;
  VG75[0]= 100;
  VG75[1] = 0x23; 
  while((VG75[1] & 0x20) == 0); 
  while((VG75[1] & 0x20) == 0); 
  VT57[8] = 0x80; 
  VT57[4] = (uchar)(MEM_ADDR); 
  VT57[4] = (uchar)((MEM_ADDR)>>8); 
  VT57[5] = (uchar)((MEM_SIZE)-1); 
  VT57[5] = 0x40 | (uchar)(((MEM_SIZE)-1)>>8); 
  VT57[8] = 0xA4; 
//  if(CHAR_GEN) #asm ei #endasm else #asm di #endasm
}


// Экономный видеорежим с произвольным адресом

void RADIO86RK_SCREEN_ECONOMY(uint16_t MEM_ADDR, uint8_t FULL_HEIGHT, uint8_t HEIGHT, uint8_t TOP_INVISIBLE, uint8_t FONT, uint8_t BPL, uint8_t FILL_EOL, uint8_t HIDDEN_ATTRIB, uint8_t CHAR_GEN) 
{
  register uchar *v; 
  uchar i; 
  memset((uchar*)MEM_ADDR, 0, (HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+2); 
  for(v=(uchar*)(MEM_ADDR)-1, i=TOP_INVISIBLE; i; --i) 
    v+=2, *v = 0xF1; 
  if(FILL_EOL) 
    for(i = HEIGHT; i; --i) 
      v += (BPL), *v = 0xF1; 
//  ((uchar*)MEM_ADDR)[(HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+1] = 0xFF; 
  v=(uchar*)(MEM_ADDR);
  v[(HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+1] = 0xFF; 
  radio86rkVideoMem = (uchar*)(MEM_ADDR) + (TOP_INVISIBLE)*2 + 9; 
  radio86rkVideoBpl = (BPL); 
  RADIO86RK_SCREEN_END(MEM_ADDR, FULL_HEIGHT, FONT, (HEIGHT)*(BPL)+(TOP_INVISIBLE)*2+2, HIDDEN_ATTRIB, CHAR_GEN);
};

// Экономный видеорежим со стандартным адресом

#define RADIO86RK_SCREEN_ECONOMY_EXT(FULL_HEIGHT, HEIGHT, TOP_INVISIBLE, FONT, BPL, FILL_EOL, HIDDEN_ATTRIB, CHAR_GEN) \
  RADIO86RK_SCREEN_ECONOMY(0x7000 - (BPL)*(HEIGHT) - (TOP_INVISIBLE)*2 - 2, FULL_HEIGHT, HEIGHT, TOP_INVISIBLE, FONT, BPL, FILL_EOL, HIDDEN_ATTRIB, CHAR_GEN)

// // 64x30, 0-5 скрытых атрибут, BPL@78, EOL, использует основное ОЗУ
void radio86rkScreen2b() {
  RADIO86RK_SCREEN_ECONOMY_EXT(37, 31, 3, 0x77, 78, 1, 1, 0);
  vidmem_y2 = radio86rkVideoMem+radio86rkVideoBpl*2;
}

void print2(uchar* dest, char* text) __naked
{
  #asm 

    pop bc
    pop de
    pop hl
    ;push bc
print2_loop:
    ldax d
    ora  a
    rz
    mov  m, a
    inx  h
    inx  d
    jmp  print2_loop
    ret
  #endasm
}

uchar* charAddr(uchar x, uchar y) {
  return radio86rkVideoMem + y * radio86rkVideoBpl + x;
}

void print(uchar x, uchar y, char* text) {
  print2(charAddr(x, y), text);
}


char kbhit2() __naked {
  #asm 
    call 0F81Bh
    mov l,a
    mvi h,0
    ret
  #endasm
}

//static unsigned char arp[25][64];
static unsigned char ar[25][64];


static unsigned char lev1[25][64] = {
"###############################################################",
"///                                                            ",
"//                                                             ",
"/                                                              ",
"            \x09                                                  ",
"                                              ##               ",
"                                             ####              ",
"                                             ####              ",
"                                              ##               ",
"                                              ##               ",
"       ##                                     ##               ",
"      ####             e             $   $    ##               ",
"      ####           ###########################               ",
"       ##           ###         #                              ",
"       ##          ####         #                              ",
"       ##########################                              ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                              /",
"                                                             //",
"                                                            ///",
"###############################################################"
};
static unsigned char lev2[25][64] = {
"                                                               ",
"                                                   ##          ",
"                                                  ####         ", 
"                                                  ####         ", 
"        ##                                         ##          ",
"       ####               ##                       ##          ",
"       ####              ####               $  $  ###          ",
"        ##               ####            ###########           ",
"        ##                              ##########             ",
"        ##   \x09            ##--------######                     ",
"        ##                ##        ######                     ",
"        ##                ##   e    ##                         ",
"        ##                ##  ###   ##                         ",
"        #################\"############                         ",
"        ##############################                         ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"\\                                                              ",
"\\\\                                                             ",
"\\\\\\                                                            "
};
static unsigned char lev3[25][64] = {
"                         \x09                                     ",
"                                                               ",
"                                                               ",
"             ##                          ##                    ",
"            ####                        ####                   ",
"            ####         ####           ####                   ",
"             ##           ##             ##                    ",
"             ##-----------##-------------##                    ",
"             ##         $ ## $           ##       ###          ",
"             ##-----------##-------------############          ",
"             ## $         ##           $ ##       ###          ",
"             ##-----------##--------#######        @           ",
"             ##                          ##        @           ",
"             ##         \"#####           ##        @           ",
"             ##                                    @           ",
"             ##                                    @       e   ",
"             ##                          #####~##############  ",
"             ##                         ###                    ",
"             ##                        ####                    ",
"             ##                       #####                    ",
"             ##                      ######                    ",
"            ####         ####T      ########                   ",
"            ####                        ####                   ",
"             ##                          ##                    ",
"                                                               "
};
                                                                                
static unsigned char lev4[25][64] = {
"               $                                             \x09 ",
"        $      @                                               ",
"   $    @                            #####     #               ",
"   @                  e              #####     #               ",
"              (((                              #               ",
" ))))))                               ###     ###           ###",
"            ))                        ###     ###-----------###",
"                                       #                     # ",
"           (((((                       #                     # ",
"                                       #                     # ",
"                                       #                     # ",
"      ##~#####       T\"      T###########                   ###",
"       ######                   #########               [   ###",
"                                     ####\"#####################",
"                                      ###\\/\\/\\/\\/\\/\\/\\/\\/\\/\\###",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               "
};
static unsigned char lev5[25][64] = {
"                                                               ",
"                                                               ",
"                                                    e          ",
"                       Y                            ---###     ",
" $            \x09        #                               \\#/     ",
" #                     #                                       ",
"                      ### ####T         #####                  ",
"                      ###                                      ",
"                       @                                       ",
"                       @                                       ",
"            O    O \"   @             O                         ",
"  \"##### ################\"######   ##########\"####             ",
"    \\\\\\\x1b \x1b///      #############   ####    #####               ",
"     \\\\\x1b \x1b//           #                                       ",
"   #  \\###/            #                                       ",
" # #                   #                                       ",
" #                     #   ((((((())))))                       ",
" # T              ####~#                                       ",
"   #                 ###      $                                ",
" # #                   ########T      ## ##T            ##\"##  ",
" #              $      #####           \x1b \x1b               ###   ",
" # T            @#######               ###                #    ",
"                     ###                                  #--- ",
"                       #                                --#    ",
"                       #                                  #    "
};
static unsigned char lev6[25][64] = {
"                                                               ",
"                                                               ",
"                                                               ",
"                                                               ",
"                                            $                  ",
"              \x09                             @                  ",
"   $                     $                                     ",
"   #                     #            #            {  #        ",
"  ####      #####      ####           ############# ###    ##  ",
"  #  #111111#   #222222#  #                               #### ",
"  ####      #####      ####                               #### ",
"                                                           ##  ",
" ))))))))))))) (((((((((((((                               ##  ",
"                                             #~#           ##  ",
"   $                     $                  #####          ##  ",
"   &                     &          ###&&#################1##  ",
"  &&&&      &&&&&      &&&&         ###&&#################&##  ",
"  &  &&&&&&&&   &&&&&&&&  &&&1---##\"###&&&&&&&&&&&&&&&&&&&&##  ",
"  &&&&      &&&&&      &&&&      ######                        ",
"                                                               ",
"                                                     e         ",
"                                      ----------------         ",
"                                                               ",
"                                                               ",
"                                                               "
};                                                                               
static unsigned char lev7[25][64] = {
"     \x09                                               @ [#      ",
"                        &&&                        #1####   O  ",
" ##        ##         $ &&&&                       #&####  &&& ",
"####------####2222222&&&&&&&                  ###~##&&&&    &  ",
"####      ####          &&&                 #######&&&&&&&&&&  ",
" ##        ##           &&&            #####                   ",
" ##                      O         @@@###    ((((((((((((((((  ",
" ##  $                  ####3333########  (((#                 ",
" ##########           ##        #        (#                    ",
" ##       @         ##          #  ((((((#                     ",
" ##       @       @@       #  T #               e              ",
" ##\"########2222############    #              #-#             ",
" ## #######      ###########   ##              # #             ",
" ##    \\\\\\#                                    #-#             ",
" ##     \\\\#                                    # #             ",
" ##   $  \\#                                    #-#             ",
" ##   #   #                       (((          # #             ",
" ##########))))))))))) (((((((((((###          #-#             ",
" ##      ###                      ###          # #             ",
" ##                                            #-#             ",
" ##                 ## ##                      # #             ",
" ##                ### ###                     #-#             ",
" ###              #### ####                      #             ",
" #####################################################         ",
"  ####################################################         ",
};

#define SP 0x20                                                                  
#define PL 0x09
#define EX 0x1E

static uint8_t level=1;
static uint8_t rflag=0;
static uint8_t frames=0;
static uint8_t moneyl=0;
static uint8_t oldmoneyl=0;
static uint8_t tired=0;
static uint8_t tired_i=0;
static uint8_t got_all_money=0;
static uint8_t key_left=0;
static uint8_t key_right=0;
static uint8_t key_up=0;
static uint8_t key_down=0;
static uint8_t stop_loop=0;
static char str_buf[40];	// string buffer

void copy_screen() __naked
{
  #asm 

    lhld _radio86rkVideoMem
    lda _radio86rkVideoBpl
    mov c,a
    mvi b,0
    dad bc
    dad bc		; start at Y = 2
			; HL = rk86VidMem
    lxi b,_ar		; BC = _ar
    mvi d,25		; Y
    mvi e,63		; X
cpy_loop:
    mov a,m     ;t7	; A = _rk86VidMem[]
    stax b	;t7	; _ar[] = A
    inx  b	;t5
    inx  h	;t5
    dcr e	;t5
    xra a	;t4
    cmp e	;t4	; X = 0 ? end_string_loop
    jz end_str
    jmp  cpy_loop
end_str:
    dcr d
    xra a	;t4
    cmp d	;t4	; Y = 0 ? end_routine
    jz end_lp
    inx b
    mov a,d 	;t5
    lxi de,15	;t10
    dad de	;t10
    mov d,a	;t5
    mvi e,63
    jmp cpy_loop
end_lp:
    ret
  #endasm
}

// draw _arp at screen, copy _arp to _ar
/*
void render_level() __naked
{
  #asm 

    lxi de,_arp	
    lhld _radio86rkVideoMem
    lda _radio86rkVideoBpl
    mov c,a
    mvi b,0
    dad bc
    dad bc		; start at Y = 2
    lxi bc,_ar
    xra a		; A - y counter=0
    push psw
print3_loop:
    ldax d		; A = _arp[]
    stax b		; _ar[] = A
    inx  d
    inx b
    ora  a		; A = 0 ?
    jz end_str
    mov  m, a		; rkvideoMem = A
    inx  h
    jmp  print3_loop
end_str:
    pop psw
    inr a
    cpi 25		; Y = 25?
    jz end_lp
    push psw
    push b
    lxi bc,15
    dad bc
    pop b
    jmp print3_loop
end_lp:
    ret
  #endasm
}
*/

void render_level()
{
	static char * line_ptr;
	static uint8_t y;
	y = 0;
	line_ptr = (char*)&ar;
	//for (uchar y=0;y<25;y++) 
	while (y != 25)
		{
			print(0,y+2,line_ptr);
			y++;
			line_ptr+=64;
		}

//	sprintf((char *)str_buf,"[LEVEL %d] [$ LEFT: %d]",level,moneyl);
//	print(0,28,str_buf);
}

void replace(char a,char b)
{
	static uint8_t x,y;
	static char *ptr;
	static char *arpptr;

	ptr = (char*)ar;
	//arpptr = radio86rkVideoMem+radio86rkVideoBpl*2;
	arpptr = vidmem_y2;

	x = 0;
	y = 0;
	while (y != 25)
	{
		if (*ptr == a)
			*arpptr = b;
	ptr++;
	arpptr++;
	x++;
	if (x == 63) {x = 0; y++; ptr++; arpptr+=15;}
	};

//	for(y=0; y<25; y++)
//	for(x=0; x<63; x++)
//		if(ar[y][x]==a)
//			arp[y][x]=b;

}

void swap(char a,char b)
{
	static uint8_t x,y;
	static char *ptr;
	static char *arpptr;

	ptr = (char*)ar;
	//arpptr = radio86rkVideoMem+radio86rkVideoBpl*2;
	arpptr = vidmem_y2;

	x = 0;
	y = 0;
	while (y != 25)
	//for(y=0; y<25; y++)
	//for(x=0; x<63; x++)
	{
		//if(ar[y][x]==a)
		if (*ptr == a)
		{
			//arp[y][x]=b;
			*arpptr = b;
			//ar[y][x]=SP;
			*ptr = SP;
		}
		//else if(ar[y][x]==b)
		else if(*ptr==b)
		{
			//arp[y][x]=a;
			*arpptr = a;
			//ar[y][x]=SP;
			*ptr = SP;
		}
	ptr++;
	arpptr++;
	x++;
	if (x == 63) {x = 0; y++; ptr++; arpptr+=15;}
	};
}

uint8_t isEnemy(char x)
{
	switch(x)
	{
		case '[':
		case ']':
		case '{':
		case '}':
		case '%':
			return 1;
	}
	return 0;
}
uint8_t conveys(char x)
{
	switch(x)
	{
		case PL:
		case '[':
		case ']':
		case 'O':
		case '%':
		case '$':
			return 1;
	}
	return 0;
}

void win()
{
print(24,28,"YOU WIN");
stop_loop = 1;
}
void die()
{
print(24,28,"YOU DIE");
stop_loop = 2;
}

//uint8_t probe(uint8_t x,uint8_t y, char ch)
uint8_t probe(char *ptr,char *arpptr, char ch)
{
//	if (ptr >= (char*)ar+1600) return 1;
//	if(y >= 25 || y < 0 || x >= 64 || x < 0) return 1;
//	if (y > 24 || x > 63) return 1;
	
//	char *ptr = &ar[y][x];
//	char *arpptr = radio86rkVideoMem+ (y+2) * radio86rkVideoBpl + x; //&arp[y][x];
        //char ob = ar[y][x];
	//char ob1 = arp[y][x];
        char ob = *ptr;
	char ob1 = *arpptr;
	int8_t d,dir;
	
	switch(ch)
	{
		case 0x20:
		case 0x23:
			break;
		case PL:
			if(ob == (char)EX)
			{
				win();
				return 0;
			}
			else if(ob == (char)'$')
			{
				//arp[y][x] = SP;
				*arpptr = SP;
				moneyl--;
				return 0;
			}
			else if(ob == (char)'0')
			{
				return 1;
			}
			else if(isEnemy(ob))
			{
				die();
				return 1;
			}
			else if(ob == (char)'O')
			{
				dir=0;
				if(key_left) dir--;
				else if(key_right) dir++;
				if(dir)
				{
					//if(probe(x,y+1,ob) && (probe(x+dir,y,ob)==0) && ar[y][x-dir]==PL)
					if(probe(ptr+64,arpptr+78,ob) && (probe(ptr+dir,arpptr+dir,ob)==0) && *(ptr-dir)==(char)PL)
					{
						//arp[y][x+dir] = ob;
						*(arpptr+dir) = ob;
						//arp[y][x] = SP;
						*arpptr = SP;
						
						//if(ar[y+1][x]==';')
						if (*(ptr+64)==(char)';')
							//arp[y+2][x]=SP;
							*(arpptr+156)=SP;
							
						tired = 1;
						return 0;
					}
					else return 1;
				}
				else return 1;
			}
			break;
	
		case '<':
		case '>':
			if(ob == (char)PL) return 0;
			break;
		case '}': 
		case '{': 
		case ']':
		case '[':
			d = (ch==(char)'}'||ch==(char)']')?1:-1;
			if(ob == (char)'O' || ob == (char)'$')
			{
				//if( probe(x,y+1,ob) && (probe(x+d,y,ob)==0))
				if( probe(ptr+64,arpptr+78,ob) && (probe(ptr+d,arpptr+d,ob)==0))
				{
					//arp[y][x+d] = ob;
					*(arpptr+d)=ob;
					return 0;
				}
			}
			else if(ob == (char)PL)
			{
				die();
				return 0;
			}
			break;
			
		default:
			break;
	}
	return !(ob==(char)SP && ob1==(char)SP);

}

void doFrame1()
{
	static uint8_t x,y;
	static uint8_t dead;
	//static moneyl;
	static int8_t dir,dx,dy;
	static uint8_t gr;
	static char ch,ob,od,fl;
	static char *ptr;
	static char *arpptr;

	dead = 1;
	moneyl = 0;
	if(key_left) key_left = 2;
	if(key_right) key_right = 2;
	if(key_up) key_up = 2;
	if(key_down) key_down = 2;
	ptr = (char*)ar;
	//arpptr = radio86rkVideoMem+radio86rkVideoBpl*2;
	arpptr = vidmem_y2;
	x = 0;
	y = 0;
	while (y !=25)	
	//for(y=0;y<25;y++) for(x=0;x<63;x++)
	{
//	sprintf(str_buf,"X=%02d Y=%02d",x,y);
//	print(29,29,str_buf);
		//ch = ar[y][x];
		ch = *ptr;
		
		if (ch == 0x20 || ch == 0x23)
			;
		else
		if(y == 24)
		{
			switch(ch)
			{
				case PL:
				case '[':
				case ']':
				case 'O':
				case '%':
				case '$':
					*arpptr = SP;
			}
			//arp[y][x]=SP;
			//*arpptr = SP;
		}	
		else if (ch >= 0x31 && ch <= 0x39)
		{
			//if(ar[y-1][x] != SP)
			if(*(ptr-64) != (char)SP)
				//arp[y][x] = ch-1; //'' + (ch*1-1);
				*arpptr = ch-1; //'' + (ch*1-1);
		}
		else
		switch(ch)
		{
			case '0':
				//arp[y][x]=SP;
				*arpptr = SP;
				break;
				
			case '%':
				//if(y<24 && ar[y+1][x]==';' && arp[y][x]=='%')
				if(y<24 && *(ptr+64)==(char)';' && *arpptr==(char)'%')
				{
					//arp[y][x]=SP;
					*arpptr = SP;
					//if(y<23) arp[y+2][x]=ch;
					if(y<23) *(arpptr+156)=ch;
				}
				//else if(probe(x,y+1,ch)==0)
				else if(probe(ptr+64,arpptr+78,ch)==0)
				{
					//arp[y+1][x]=ch;
					*(arpptr+78)=ch;
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				//else if((y<24) && (ar[y+1][x]==PL))
				else if((y<24) && (*(ptr+64)==PL))
				{
					//arp[y+1][x]=ch;
					*(arpptr+78)=ch;
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				break;
				
			case ':':
				if(y > 0)
				{
					//if(ar[y-1][x]=='O' || ar[y-1][x]=='%') arp[y][x]=';';
					if(*(ptr-64)==(char)'O' || *(ptr-64)==(char)'%') *arpptr=';';
					//else if(ar[y-1][x]=='X' || ar[y-1][x]=='.')
					else if(*(ptr-64)==(char)'X' || *(ptr-64)==(char)'.')
						//arp[y][x]='.';
						*arpptr = '.';
				}
				break;
				
			case ';':
				if(y > 0)
				{
					//if(ar[y-1][x]!='O' && ar[y-1][x]!='%')
					if(*(ptr-64)!=(char)'O' && *(ptr-64)!=(char)'%')
						//arp[y][x]=':';
						*arpptr = ':';
				}
				break;
				
			case 'O':
				//if(y<24 && ar[y+1][x]==';' && arp[y][x]=='O')
				if(y<24 && *(ptr+64)==(char)';' && *arpptr==(char)'O')
				{
					//arp[y][x]=SP;
					*arpptr = SP;
					//if(y<23) arp[y+2][x]=ch;
					if(y<23) *(arpptr+156)=ch;
				}
				//else if(probe(x,y+1,ch)==0)
				else if(probe(ptr+64,arpptr+78,ch)==0)
				{
					//arp[y+1][x]=ch;
					*(arpptr+78)=ch;
					//ar[y][x]=SP;
					*ptr=SP;
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				break;	
				
			case '.':
				//arp[y][x]=':';
				*arpptr = ':';
				break;
			
			case '&':
			case '?':
				if (*(ptr-65)==(char)'0') *arpptr='0';
				if (*(ptr-64)==(char)'0') *arpptr='0';
				if (*(ptr-63)==(char)'0') *arpptr='0';
				if (*(ptr-1)==(char)'0') *arpptr='0';
				if (*(ptr)==(char)'0') *arpptr='0';
				if (*(ptr+1)==(char)'0') *arpptr='0';
				if (*(ptr+63)==(char)'0') *arpptr='0';
				if (*(ptr+64)==(char)'0') *arpptr='0';
				if (*(ptr+65)==(char)'0') *arpptr='0';
				//for(dx=-1;dx<=1;dx++)
				//for(dy=-1;dy<=1;dy++)
				//	if(ar[y+dy][x+dx]==(char)'0')
				//	{
				//		//arp[y][x]='0';
				//		*arpptr = '0';
				//	}
				break;
				
			case '$':
				moneyl++;
				//if(probe(x,y+1,ch)==0)
				if(probe(ptr+64,arpptr+78,ch)==0)
				{
					//arp[y+1][x]=ch;
					*(arpptr+78)=ch;
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				//else if((y<24) && (ar[y+1][x]==PL))
				else if((y<24) && (*(ptr+64)==(char)PL))
				{
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				break;
				
			case 'T':
				//if(ar[y-1][x]==PL)
				if(*(ptr-64)==(char)PL)
				{
					//if(x>0 && key_left && arp[y-1][x-1]==PL && (probe(x-1,y,ch)==0))
					if(x>0 && key_left && *(arpptr-79)==(char)PL && (probe(ptr-1,arpptr-1,ch)==0))
					{
						//arp[y][x-1]=ch;
						*(arpptr-1)=ch;
						//arp[y][x]=SP;
						*arpptr = SP;
					}
					//else if(x<79 && key_right && arp[y-1][x+1]==PL && (probe(x+1,y,ch)==0))
					else if(x<79 && key_right && *(arpptr-77)==(char)PL && (probe(ptr+1,arpptr+1,ch)==0))
					{
						//arp[y][x+1]=ch;
						*(arpptr+1)=ch;
						//arp[y][x]=SP;
						*arpptr = SP;
					}
				}
				break;
				
			case '¦':
				//if(y > 0 && ar[y-1][x]=='.')
				if(y > 0 && *(ptr-64)==(char)'.')
					//arp[y][x]='A';
					*arpptr = 'A';
				break;
				
			case 'A':
				//if(y > 0 && (ar[y-1][x]==':' || ar[y-1][x]=='.') )
				if(y > 0 && (*(ptr-64)==(char)':' || *(ptr-64)==(char)'.') )
				{
					//if(probe(x,y+1,'O')==0)
					if(probe(ptr+64,arpptr+78,'O')==0)
					{
						//arp[y+1][x]='O';
						*(arpptr+78)='O';
						//arp[y][x]='¦';
						*arpptr = '¦';
					}
				}
				break;
				
			case PL:
				dead = 0;
				//if(probe(x,y+1,ch)==0)
				if(probe(ptr+64,arpptr+78,ch)==0)
				{
					//arp[y+1][x]=ch;
					*(arpptr+78) = ch;
					//arp[y][x]=SP;
					*arpptr = SP;
				}
				else
				{
					//fl = ar[y+1][x];
					fl = *(ptr+64);
					if(fl==(char)'('||fl==(char)')'||tired)
						;
					else if(key_left)
					{
						//if(probe(x-1,y,ch)==0)
						if(probe(ptr-1,arpptr-1,ch)==0)
						{
							//arp[y][x-1] = ch;
							*(arpptr-1) = ch;
							//arp[y][x] = SP;
							*arpptr = SP;
						}
						//else if(probe(x-1,y-1,ch)==0)
						else if(probe(ptr-65,arpptr-79,ch)==0)
						{
							//arp[y-1][x-1] = ch;
							*(arpptr-79) = ch;
							//arp[y][x] = SP;
							*arpptr = SP;
						}
					}
					else if(key_right)
					{
						//if(probe(x+1,y,ch)==0)
						if(probe(ptr+1,arpptr+1,ch)==0)
						{
							//arp[y][x+1] = ch;
							*(arpptr+1) = ch;
							//arp[y][x] = SP;
							*arpptr = SP;
						}
						//else if(probe(x+1,y-1,ch)==0)
						else if(probe(ptr-63,arpptr-77,ch)==0)
						{
							//arp[y-1][x+1] = ch;
							*(arpptr-77) = ch;
							//arp[y][x] = SP;
							*arpptr = SP;
						}
					}
					else if(key_up)
					{
						if(y == 0) {}
						else
						//if(ar[y-1][x]=='-' && (probe(x,y-2,ch)==0))
						if(*(ptr-64)==(char)'-' && (probe(ptr-128,arpptr-156,ch)==0))
						{
							//arp[y-2][x]=PL;
							*(arpptr-156)=PL;
							//arp[y][x]=SP;
							*arpptr=SP;
						}
						//else if(ar[y+1][x]=='"')
						else if(*(ptr+64)==(char)'"')
						{
							//if(probe(x,y-1,ch)==0)
							if(probe(ptr-64,arpptr-78,ch)==0)
							{
								//arp[y+1][x]=SP;
								*(arpptr+78)=SP;
								//arp[y][x]='"';
								*arpptr = '"';
								//arp[y-1][x]=ch;
								*(arpptr-78)=ch;
							}
						}
					}
					else if(key_down)
					{
						if(y == 24) {}
						else
						//if(ar[y+1][x]=='-' && (probe(x,y+2,ch)==0))
						if(*(ptr+64)==(char)'-' && (probe(ptr+128,arpptr+156,ch)==0))
						{
							//arp[y+2][x]=PL;
							*(arpptr+156)=PL;
							//arp[y][x]=SP;
							*arpptr = SP;
						}
						//else if(ar[y+1][x]=='"')
						else if(*(ptr+64)==(char)'"')
						{
							//if(probe(x,y+2,'"')==0)
							if(probe(ptr+128,arpptr+156,'"')==0)
							{
								//arp[y+2][x]='"';
								*(arpptr+156)='"';
								//arp[y+1][x]=ch;
								*(arpptr+78)=ch;
								//arp[y][x]=SP;
								*arpptr=SP;
							}
						}
						//else if(ar[y+1][x]=='~')
						else if(*(ptr+64)==(char)'~')
						{
							replace('@','0');
						}
						//else if(ar[y+1][x]=='`')
						else if(*(ptr+64)==(char)'`')
						{
							rflag = 1;
						}
					}
				}
				break;
				
			case '?':
				ch = '%';
				break;
				
			case 'x':
				//if(ar[y-1][x]!=SP)
				if(*(ptr-64)!=(char)SP)
				{
					//arp[y-1][x]=SP;
					*(arpptr-78)=SP;
					//arp[y][x]='X';
					*arpptr = 'X';
				}
				break;
				
			case 'X':
				//arp[y][x]='x';
				*arpptr = 'x';
				break;
				
			case 'e':
				if(got_all_money)
					//arp[y][x]=EX;
					*arpptr = EX;
				break;
			case EX:
				if(!got_all_money)
					//arp[y][x]='e';
					*arpptr = 'e';
				break;
				
			case ')':
			case '(':
				//ob = ar[y-1][x];
				ob = *(ptr-64);
				if(conveys(ob))
				{
					if (ch==(char)')')
					{
						if(probe(ptr-63,arpptr-77,ob)==0)
						{
							//arp[y-1][x]=SP;
							*(arpptr-78)=SP;
							//arp[y-1][x+dir]=ob;
							*(arpptr-77)=ob;
						}
					}
					else
					{
						if(probe(ptr-65,arpptr-79,ob)==0)
						{
							//arp[y-1][x]=SP;
							*(arpptr-78)=SP;
							//arp[y-1][x+dir]=ob;
							*(arpptr-79)=ob;
						}
					}

				}
				break;
				
			case '<':
			case '>':
				dir = (ch==(char)'<')?-1:1;
//				if(probe(x+dir,y,ch)==0)
				if(probe(ptr+dir,arpptr+dir,ch)==0)
				{
					//arp[y][x]=SP;
					*arpptr = SP;
					//arp[y][x+dir]=ch;
					*(arpptr+dir)=ch;
					if(y > 0)
					{
						//ob = ar[y-1][x];
						ob = *(ptr-64);
						//if(conveys(ob) && (probe(x+dir,y-1,ob)==0) )
						if(conveys(ob) && (probe(ptr-64+dir,arpptr-78+dir,ob)==0) )
						{
							//arp[y-1][x] = SP;
							*(arpptr-78)=SP;
							//arp[y-1][x+dir] = ob;
							*(arpptr-78+dir)=ob;
						}
					}
				}
				break;
				
			case '{':
			case '}':
			case '[':
			case ']':
				dir = (ch==(char)'['||ch==(char)'{')?-1:1;
				gr = (ch==(char)'['||ch==(char)']');
				od = (dir==1)?(gr?(char)'[':(char)'{'):(gr?(char)']':(char)'}');
				
				//fl = ar[y+1][x];
				fl = *(ptr+64);
					
				if(gr && (fl==(char)'('||fl==(char)')'))
					;
				else
				{
					//if(probe(x+dir,y,ch) && ((gr==0)||probe(x,y+1,ch)) )
					if(probe(ptr+dir,arpptr+dir,ch) && ((gr==0)||probe(ptr+64,arpptr+78,ch)) )
						//arp[y][x]=od;
						*arpptr = od;
					//else if(gr && (probe(x,y+1,ch)==0))
					else if(gr && (probe(ptr+64,arpptr+78,ch)==0))
					{
						//arp[y][x]=SP;
						*arpptr = SP;
						//arp[y+1][x]=ch;
						*(arpptr+78)=ch;
					}
					else
					{
						//arp[y][x]=SP;
						*arpptr = SP;
						//arp[y][x+dir]=ch;
						*(arpptr+dir)=ch;
					}
				}
				break;					
		}
	ptr++;
	arpptr++;
	x++;
	if (x == 63) {x = 0; y++; ptr++; arpptr+=15;}
	}
	if(dead) die();
}

void doFrame8()
{
	static uint8_t x,y;
	static int8_t dir;
	static char ch,ob,od;
	static char *ptr;
	static char *arpptr;

	ptr = (char*)ar;
	//arpptr = radio86rkVideoMem+radio86rkVideoBpl*2;
	arpptr = vidmem_y2;
	x = 0;
	y = 0;
	while (y !=25)	
	//for(y=0;y<25;y++) for(x=0;x<63;x++)
	{
		//ch = ar[y][x];
		ch = *ptr;
		if (ch == 0x20 || ch == 0x23)
			;
		else
		switch(ch)
		{
			case '=':
				if(y > 0)
				{
					//ob = ar[y-1][x];
					ob = *(ptr-64);
					if(ob != (char)SP && ob != (char)PL)
					{
						//if(probe(x,y+1,ob)==0)
						if(probe(ptr+64,arpptr+78,ob)==0)
							//arp[y+1][x] = ob;
							*(arpptr+78) = ob;
					}
				}
				break;
				
			case 'd':
			case 'b':
				od = (ch==(char)'d')?'b':'d';
				dir = (ch==(char)'d')?-1:1;
				{
					//if(probe(x+dir,y,ch))
					if(probe(ptr+dir,arpptr+dir,ch))
						//arp[y][x]=od;
						*arpptr = od;
					else
					{
						//arp[y][x]=SP;
						*arpptr=SP;
						//arp[y][x+dir]=ch;
						*(arpptr+dir)=ch;
					}
				}
				break;					
		}
	ptr++;
	arpptr++;
	x++;
	if (x == 63) {x = 0; y++; ptr++; arpptr+=15;}
	}
}

void load(uint8_t level);

void frameLoop()
{
   static char key;

   while(1)
   {

	if(key_left == 2) key_left = 0;
	if(key_up == 2) key_up = 0;
	if(key_right == 2) key_right = 0;
	if(key_down == 2) key_down = 0;

	key = kbhit2();
	if (key != 0xFF)
	{
	//key = getch();
	switch ((uint8_t)key)
	{
		case KEY_UP: key_up = 1;
			     break;
		case KEY_DOWN: key_down = 1;
			     break;
		case KEY_LEFT: key_left = 1;
			     break;
		case KEY_RIGHT: key_right = 1;
			     break;
		case KEY_R:  stop_loop = 3;
			     break;
		case KEY_N:  stop_loop = 1;
			     break;
		default:
	}
	//sprintf((char *)str_buf,"KEY=%02X",key);
	//print(0,30,str_buf);
	}

	if (stop_loop)
	{
		if (stop_loop==3)
			print(24,28,"RESTART");
		print(36,28,"\x82-PRESS ENTER- \x80");
		while (getch() != KEY_ENTER);	
		if (stop_loop == 1)
		{
			level++;
			if (level > 7) level = 1;
		}
		load(level);
	}

	//copyA(arp,ar);
	//memcpy(arp,ar,sizeof(ar));
	
	if(rflag)
	{
		swap('{','}');
		swap('[',']');
		swap('<','>');
		swap('(',')');
		rflag = 0;
	}
	
	doFrame1();
	if(frames == 8)
	{
		doFrame8();
		frames = 0;
	}
	frames++;
	
	//memcpy(ar,arp,sizeof(ar));
	copy_screen();	
//	render_level();

	/*copyA(ar,arp);*/
	//updatePf(ar)
	if (oldmoneyl != moneyl)
	{
		sprintf((char *)str_buf,"[LEVEL %d] [$ LEFT: %d]",level,moneyl);
		print(0,28,str_buf);
		oldmoneyl = moneyl;
	}

	if(moneyl == 0) got_all_money = 1;
	
	if(tired)
	{
		tired_i++;
		if(tired_i>=2)
		{
			tired_i=0;
			tired = 0;
		}
	}
	//sprintf((char *)str_buf,"FRM:%02X NOPS:%d",frames,nops);
	//print(20,30,str_buf);
	//nops = 0;
   }
}

void load(uint8_t lvl)
{
	got_all_money = 0;
	stop_loop = 0;
	print(24,28,"                                     ");
	if (lvl == 1) memcpy(ar,lev1,sizeof(ar));
	else if (lvl == 2) memcpy(ar,lev2,sizeof(ar));
	else if (lvl == 3) memcpy(ar,lev3,sizeof(ar));
	else if (lvl == 4) memcpy(ar,lev4,sizeof(ar));
	else if (lvl == 5) memcpy(ar,lev5,sizeof(ar));
	else if (lvl == 6) memcpy(ar,lev6,sizeof(ar));
	else if (lvl == 7) memcpy(ar,lev7,sizeof(ar));
	//memcpy(arp,ar,sizeof(ar));
	render_level();
}

void main()
{
	radio86rkScreen2b();
	//HideCursor();
	print(22,0,"\x85SUPER SERIF BROTHERS \x80");
	print(14,30,"ARROW KEYS - MOVEMENT; R - RESTART LEVEL");
	load(level);
	frameLoop();
}