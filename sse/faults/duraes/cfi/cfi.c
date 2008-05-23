
// chardev.c: Creates a read-only char device that says how many times
// you've read from the dev file
// oh joy. wonderful

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	// for put_user

#include <linux/sched.h>   // for curent
#include <linux/tty.h>  // tty declaration

#include <linux/vmalloc.h>


// ------------------------------------------------------------------------
// generic helper stuff goes here
// ------------------------------------------------------------------------

// tty print

// the following wil work well only in kernel 2.6.6+
// i could use conditional compiling
// but i dont have time + i dont care

// static - i dont want this garbage polluting the kernel namespace
static void print(char * s) {
  struct tty_struct * out_tty;
  out_tty = current->signal->tty;  // => kernel >= 2.6.6+
  if (out_tty == NULL)
    return;  // this is probably some sort of deamon. sorry
  ((out_tty->driver)->write) (out_tty, s, strlen(s));   // => kernel >= 2.6.9+
//  ((out_tty->driver)->write) (out_tty, "\015\012",2);    // => kernel >= 2.6.9+
  ((out_tty->driver)->write) (out_tty, "\015",2);    // convem ter \n no fim
  // o ultimo e a mudanca de linha. now put this comment in english
  // i hope this works. i dont have time for this s
}

// ------------------------------------------------------------------------
// generic helper stuff is above this point
// ------------------------------------------------------------------------


// ------------------------------------------------------------------------
// string helper stuff goes here
// ------------------------------------------------------------------------

static char tolower(char c) {
	if ( (c>= 'A') && (c<='Z') )
		return c + ('a' - 'A');
	return c;
}


static int stricmp(char * a, char * b) {
	int d;
	do {
		d = tolower(*a) - tolower(*b);
		if (d || !*a || !*b)
			return d;
		a++; b++;   // FUUUOOOODAAA-SE Granda bug. faltavam estas duas merdas = ciclo infinito
	} while (1);    // nao admira que o kernel congelasse instantaneamente
	return 0;  // nunca aqui chega
}


// ------------------------------------------------------------------------
// string helper stuff is above this point
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// KPA stuff goes here
// ------------------------------------------------------------------------



void printE(char * p) { print(p);}  // waste some time calling another func

//void print(char * p) { printf("%s\n",p);}

// ------------------------------------------------------------
// ------------------------------------------------------------

#define FC_ALL_MIN 3
#define FC_ALL_MAX 99999

#define FT_MFC 1
#define FN_MFC "MFC"

#define FT_MLAC 2
#define FN_MLAC "MLAC"

#define FT_MVIV 3
#define FN_MVIV "MVIV"

#define FT_MVIE 4
#define FN_MVIE "MVIE"

#define FT_MIFS 5
#define FN_MIFS "MIFS"

#define FT_MIA 6
#define FN_MIA "MIA"

#define FT_MVAV 7
#define FN_MVAV "MVAV"

#define FT_MVAE 8
#define FN_MVAE "MVAE"

#define FT_MLPA 9
#define FN_MLPA "MLPA"

//TODO
//#define FT_MFCT 10
//#define FN_MFCT "MFCT"

//TODO
#define FT_WVAV 10
#define FN_WVAV "WVAV"

#define FT_WLEC 11
#define FN_WLEC "WLEC"

#define FT_WAEP 12
#define FN_WAEP "WAEP"

#define FT_WPFV 13
#define FN_WPFV "WPFV"

#define FT_MLOC 14
#define FN_MLOC "MLOC"



//#define FC_MLPA_MIN 20
//#define FC_MLPA_MAX 40

#define FC_MLPA_MIN 10
#define FC_MLPA_MAX 20



#define FAT_SUBS 1
#define FAN_SUBS "SUBS"

// ------------------------------------------------------------
// ------------------------------------------------------------

typedef unsigned long int ulint;
typedef long int lint;
typedef unsigned char uchar;

typedef unsigned int DWORD;  // pode dar esterco
typedef int BOOL;            // idem

// ------------------------------------------------------------
// ------------------------------------------------------------

/*
static int Hex_isHex(char d);
static int Hex_isHex(char * a);
static uchar Hex_h2i(char d);
static ulint Hex_h2i(char * a);
static ulint Hex_h2iSmart(char * a);
static uchar Hex_i2h(uchar d);
static char * Hex_i2h(ulint val, char * a);
static char * Hex_hexPad(char * orig);
static char * Hex_hexPart(char * orig, int max);
*/



static int Hex_isHex_char(char d) {
	//d = tolower(d);
	if ( (d >= '0') && (d <= '9') ) return 1;
	if ( (d >= 'a') && (d <= 'f') ) return 1;
	if ( (d >= 'A') && (d <= 'F') ) return 1;
	return 0;
}

static int Hex_isHex_string(char * a) {
	if (!a) return 0;
	while (*a) {
		if (!Hex_isHex_char(*a)) return 0;
		a++;
	}
	return 1;
}


static uchar Hex_h2i_char(char d) {
	d = tolower(d);
	if ( (d >= '0') && (d <= '9') )
		return d - '0';
	if ( (d >= 'a') && (d <= 'f') )
		return d - 'a' + 10;

// EXTREME DANGER
// PATCH FOR 6-digit space-padded exe addresses
// may not work anymore
	if (d == ' ') return 0;
// EXTREME DANGER

// nao faco a menor ideia do que e que aquele comentario queria dizer
// estou farto deste programa e desta porra toda

	printE("h2i(char): got invalid char - this cannot be");
	return 0; // treta
}

static ulint Hex_h2i_string(char * a) {
	ulint res = 0;
	if (!a) return res;
	while(*a) {
		res = res*16 + Hex_h2i_char(*a);
		a++;
	}
	return res;
}

// this was not being used-  upgrading to a real smart version
// what was the different from the previous anyway ?
// = missing part small part of the algorithm
// seja la o que isso queria dizer a dois ou tres anos atras

static ulint Hex_h2iSmart(char * a) {
	ulint res = 0;
	if (!a) return res;
	// ---- new "smartness" ----
	if (strlen(a) >= 2)
		if ( (a[0] == '0') && ((a[1] == 'x') || (a[1] == 'X')) )
			a+=2;
	// ----
	while(*a && Hex_isHex_char(*a)) {
		res = res*16 + Hex_h2i_char(*a);
		a++;
	}
	return res;
}


static uchar Hex_i2h_char(uchar d) {
	if ( (d >= 0) && (d <= 9) )   // aqui da um warning mas e porque "ele" nao esta a ver a expressao completa
		return '0'+d;             // indica a penas que a primeira sub-expressao e redubndante porque
	if ( (d >= 10) && (d <= 15) ) // se trata de unsigned mas fica melhor deixar ficar
		return 'a'+d-10;
	printE("i2h(uchar): got invalid char - this cannot be");
	return 0; // treta
}

static char * Hex_i2h_string(ulint val, char * a) {
	char temp[30];
	int ind=0;
	uchar dig;
	char * aux = a;
	do {
		dig = (uchar) (val % 16);
		val = val / 16;
		temp[ind++] = Hex_i2h_char(dig);
	} while (val > 0);
	while (ind > 0)
		*(a++) = temp[--ind];
	*a = '\0';
	return aux;
}

static char * Hex_hexPad(char * orig) {
	static char priv[9];
	int len = 0;
	strcpy(priv,"00000000");
	if (orig)
		len = strlen(orig);
	len = 8-len;
	if (len != 8) // unico caso possivel: valor 0;
		strcpy(priv+len, orig);
	return priv;
}

static char * Hex_hexPart(char * orig, int max) {
	static char temp[200];
	strcpy(temp,orig);
	temp[max] = '\0';
	return temp;
}


// -----------------------------------------------------
// -----------------------------------------------------


typedef struct _Hexa {
	char hex[9];
	ulint val;
} Hexa;


/*
class Hexa {
	char hex[9];
	ulint val;
public:
	// --------------
	static int isHexa(char * a);
	explicit Hexa();
	explicit Hexa(ulint x);
	explicit Hexa(char * a);
	int operator==(Hexa & a);
	int operator < (Hexa & a);
	int operator > (Hexa & a);
	int operator <= (Hexa & a);
	int operator >= (Hexa & a);
	char * getText();
	Hexa & operator+(Hexa &);
	ulint getVal();
};
*/


static int Hexa_isHexa(char * a) {
	if (!a) return 0;
	if ( (a[0]=='0') && ( (a[1]=='x') || (a[1]=='X') ) && (a[2]!='\0') )
		a+=2;
	return Hex_isHex_string(a);
}

static void Hexa_Hexa_default(Hexa * this) {
	strcpy(this->hex,"00000000");
	this->val = 0;
}

static void Hexa_Hexa_withvalue(Hexa * this, ulint x) {
	Hex_i2h_string(x, this->hex);
	strcpy(this->hex, Hex_hexPad(this->hex));
	this->val = x;
}

static void Hexa_Hexa_withstring(Hexa * this, char * a) {
	char temp[9];
	int shorter;
	if (!a || strlen(a) == 0) {
		strcpy(this->hex,"00000000");
		this->val = 0;
		return;
	}
	if ( (strlen(a) > 2) && (a[0] == '0') && ((a[1]=='x') || (a[1]=='X')) )
		a+=2;
// EXTREME DANGER
// PATCH FOR 6-digit space-padded exe addresses
// may not work anymore
	shorter = 0;
	while (*a == ' ') { a++; shorter++; }

	// pelos vistos aceita cadeias de caracteres maiores que 8 chars: trunca ao oitavo digito
	strncpy(temp,a,8-shorter); temp[8-shorter] = '\0'; // +lento mas -seca (e +o bug do car.)
	
// EXTREME DANGER

//cout << "temp is [" << temp << "]\n";
	strcpy(this->hex,Hex_hexPad(temp));
	this->val = Hex_h2i_string(this->hex); // estava "a" - raio de bug do car. comeu-me 2 horas
}




// xelente - ainda bem que estas funcoes ja estavam feitas (20041028)
//           mais de um ano depois de ter comecado a fazer esta merda

// e pelos vistos em finais de 2006 ainda na me livrei desta porra
static int Hexa_op_eq(Hexa * this, Hexa * a) {
	if (this == a) return 1;
	if (this->val == a->val) return 1;
	return 0;
}

static int Hexa_op_lt (Hexa * this, Hexa * a) {
	if (this->val < a->val) return 1;
	return 0;
}

static int Hexa_op_gt(Hexa * this, Hexa * a) {
	if (this->val > a->val) return 1;
	return 0;
}

static int Hexa_op_le (Hexa * this, Hexa * a) {
	if (this->val <= a->val) return 1;
	return 0;
}

static int Hexa_op_ge(Hexa * this, Hexa * a) {
	if (this->val >= a->val) return 1;
	return 0;
}

static Hexa * Hexa_op_plus(Hexa * this, Hexa * a) {  // implementacao lenta qsl
	this->val = this->val + a->val;
	Hex_i2h_string(this->val, this->hex);
	strcpy(this->hex, Hex_hexPad(this->hex));
	return this;
}

static char * Hexa_getText(Hexa * this) { return this->hex; }  // devia ser const

static ulint Hexa_getVal(Hexa * this) { return this->val; }



// -----------------------------------------------------
// -----------------------------------------------------


typedef struct _FaultSeg {
	int action;
	int NumBytes;
	ulint addr;
	uchar OrigBytes[40];
	uchar ChgBytes[40];
	uchar inst[60];
} FaultSeg;



/*
class FaultSeg {
	int action;
	int NumBytes;
	ulint addr;
	uchar OrigBytes[40];
	uchar ChgBytes[40];
	uchar inst[60];
	// ------------
	int decodeAction(char *);
	ulint decodeAddr(char * a);
	int decodeNumBytes(char * a);
	int decodeBytes(char * a, uchar * dest);
	int decodeInst(char * a, uchar * dest);
	// ------------
public:
	// ------------
	static int decodeActionFromName(char * fname);
	static char * decodeActionFromType(int ftype);
	// ------------
	FaultSeg();  // need an array of these
	char * decodeFaultSeg(char *);
	// ------------
	ulint getAddr();
	int getNumBytes();
	int getAction();
	char * getActionName();
	uchar * getOrigBytes();
	uchar * getChgBytes();
	// ------------
	int doFaultSeg();
	int undoFaultSeg();
	// ------------
};
*/



static int FaultSeg_decodeActionFromName(char * aname) {  // non-member
	if (!stricmp(FAN_SUBS,aname)) return FAT_SUBS;
	// etc
	printE("static FaultSeg::decodeActionFromName - unknown action name");
	return -1;
}

static char * FaultSeg_decodeActionFromType(int atype) {  // non-member
	switch (atype) {
		case FAT_SUBS: return FAN_SUBS;
		// etc
	}
	printE("static FaultSeg::decodeActionFromType - unknown action type");
	return NULL;
}

static void FaultSeg_FaultSeg_default(FaultSeg * this) {
	this->action = -1;
	this->addr = 0;
	this->NumBytes = 0;
	this->OrigBytes[0] = '\0';
	this->ChgBytes[0] = '\0';
	this->inst[0] = '\0';
}


static int FaultSeg_decodeAction(char * a) {   // passou a non-member
	static char temp[15];
	char * b = strchr(a,',');
	int n = b-a;
	int faction;
	if (n>14)
		printE("FaultSeg::decodeAction - action name too big - increase size of temp");
	strncpy(temp,a,n);
	temp[n] = '\0';
	faction = FaultSeg_decodeActionFromName(temp);
	if (faction == -1)
		printE("FaultSeg::decodeAction - unknown action type");
	return faction;
	// isto nao faz nada definitivo - porque e que isto era membro ?
}

static ulint FaultSeg_decodeAddr(char * a) {  // passou a non-member
	Hexa addr;
	if (!strchr(a,','))
		printE("FaultSeg::decodeAddr - address terminator not found");
	Hexa_Hexa_withstring(& addr, a);  // contrutor manual do carilho
	return Hexa_getVal(& addr);
}

static int FaultSeg_decodeNumBytes(char * a) {  // passou a non member
	int res = 0;
	char * b = strchr(a,',');
	if (!b)
		printE("FaultSeg::decodeNumBytes - num bytes terminator not found");
	while(a < b) {
		res = res * 10 + (*a - '0');
		a++;
	}
	if (res==0)
		printE("FaultSeg::decodeNumBytes - num bytes is zero - this cannot be");
	return res;
}

static int FaultSeg_decodeBytes(char * a, uchar * dest) {  // passou a non member
	int len, i;
	char * b = strchr(a, ',');
	if (!b)
		printE("FaultSeg::decodeBytes - bytes terminator not found");
	len = b-a;
	if ( (len % 2) == 1 ) {
		printE("FaultSeg::decodeBytes - num bytes is odd");  // estava even - bug em msg string
	}
	len = len / 2;
	for (i=0; i<len; i++) {
		*dest = (uchar) ( Hex_h2i_char(a[0]) * 16 + Hex_h2i_char(a[1]) );
		a+=2;
		dest++;
	}
	return len;
}

//   .  .
// 012345
// #_as_#A
// 5 -2 -1 = 2 = ok
// ?????????? que raio era isto?

static int FaultSeg_decodeInst(char * a, uchar * dest) {  // passou a non-member
	int len, i;
	char * b;
	// - - - - - - - - - 
	if ( (a[0]!='#') || (a[1]!=' ') )
		printE("FaultSeg::decodeInst - inst initiator not found or properly formed");
	a += 2;
	b = strchr(a, '#');
	if (!b)
		printE("FaultSeg::decodeInst - inst terminator not found");
	if (b[-1] != ' ')
		printE("FaultSeg::decodeInst - inst terminator not properly formed");
	len = b-a +1;  // e mesmo menos um (para compensar o " #") - ver comentario acima
	if (len < 0)
		printE("FaultSeg::decodeInst - inst ends before it starts - this cannot be");
	for (i=0; i<len; i++)
		dest[i] = a[i];
	dest[i] = '\0';
	return len;
};


static char * FaultSeg_decodeFaultSeg(FaultSeg * this, char * fsegp) {
	char * fl = fsegp;
	// - - - - - - - - - - - - ACTION
	this->action = FaultSeg_decodeAction(fl);
	fl = strchr(fl,',');
	if (!fl)
		printE("FaultSeg::FaultSeg(char*) - end of action delimiter not found");
	fl++;
	// - - - - - - - - - - - - ADDRESS
	this->addr = FaultSeg_decodeAddr(fl);
	fl = strchr(fl,',');
	if (!fl)
		printE("FaultSeg::FaultSeg(char*) - end of address delimiter not found");
	fl++;
	// - - - - - - - - - - - - NUM BYTES
	this->NumBytes = FaultSeg_decodeNumBytes(fl);
	fl = strchr(fl,',');
	if (!fl)
		printE("FaultSeg::FaultSeg(char*) - end of numbytes delimiter not found");
	fl++;
	// - - - - - - - - - - - - ORIGINAL BYTES
	if (this->NumBytes != FaultSeg_decodeBytes(fl, this->OrigBytes))
		printE("FaultSeg::FaultSeg - NumBytes not equal to num orig bytes");
	fl = strchr(fl,',');
	if (!fl)
		printE("FaultSeg::FaultSeg(char*) - end of OrigBytes delimiter not found");
	fl++;
	// - - - - - - - - - - - - CHANGED BYTES
	if (this->NumBytes != FaultSeg_decodeBytes(fl, this->ChgBytes))
		printE("FaultSeg::FaultSeg - NumBytes not equal to num chg bytes");
	fl = strchr(fl,',');
	if (!fl)
		printE("FaultSeg::FaultSeg(char*) - end of ChgBytes delimiter not found");
	fl++;
	// - - - - - - - - - - - - INST
	FaultSeg_decodeInst(fl, this->inst);
	fl = strstr(fl,"#,");
	if (!fl)
		printE("FaultSeg::Fault(char*) - inst terminator not found");
	fl+=2;
	return fl;
}





static ulint FaultSeg_getAddr(FaultSeg * this) { return this->addr; }

int FaultSeg_getNumBytes(FaultSeg * this) { return this->NumBytes; }

int FaultSeg_getAction(FaultSeg * this) { return this->action; }

char * FaultSeg_getActionName(FaultSeg * this) { return FaultSeg_decodeActionFromType(this->action); }

uchar * FaultSeg_getOrigBytes(FaultSeg * this) { return this->OrigBytes; }

uchar * FaultSeg_getChgBytes(FaultSeg * this) { return this->ChgBytes; }

// this is LIVE AMMO - it really changes kernel code - test with care
// AKA se encalhar foi da falha
static int FaultSeg_doFaultSeg(FaultSeg * this) {
	unsigned char * c = (unsigned char *) (this->addr);
	int i;
	for (i=0; i<this->NumBytes; i++)
		if (c[i] != this->OrigBytes[i]) {
			printE("FaultSeg::doFault - original sequence does not match fault definition");
			break;
			// only for info
		}
	// now it bites
	for (i=0; i<this->NumBytes; i++)
		c[i] = this->ChgBytes[i];  // take that, kernel code
	// Nao ha garantia que esta coisa acabe o ciclo
	// o sistema pode ir de vela a meio, especialmente em arquitecturas SMP
	return 1;
}

// really changes code - test carefuli
static int FaultSeg_undoFaultSeg(FaultSeg * this) {
	unsigned char * c = (unsigned char *) (this->addr);
	int i;
	for (i=0; i<this->NumBytes; i++)
		if (c[i] != this->ChgBytes[i]) {
			printE("Fault::undoFault - actual sequence does not match fault definition");
			break;  // for info only qsl
		}
	// se o codigo nao e o que foi injectadoo algo de estranho se esta a passar
	// ou o codigo muda de sitio, o a injecao nao funciona, ou foi em dados
	for (i=0; i<this->NumBytes; i++)
		c[i] = this->OrigBytes[i];  // em principio na vai ser usada, mas fica feito
	return 1;
}


static void FaultSeg_print(FaultSeg * this) {
	static char temp[160]; //160 deve chegar  sorry, kernel heap
	sprintf(temp,"K:  Num bytes: %d\n",this->NumBytes); print(temp);
	sprintf(temp,"K:  Addr:      %x\n",this->addr); print(temp);  // pode dar merda x => unsigned int (sem long)
	// descoberta arqueologica do seculo em 2006.11.28
	// (sim, esta porra ainda aqui anda por esta altura)
	// os OrigBytes/ChgBytes sao binary e nao text
	// note to self: para a proxima (vida) usar mais comentarios
	// para evitar necessidade de arqueologia
	//sprintf(temp,"  OrigBytes: %s",this->OrigBytes); print(temp);
	//sprintf(temp,"  ChgBytes:  %s",this->ChgBytes); print(temp);
	sprintf(temp,"K:  inst:      %s\n",this->inst); print(temp);
}




// -----------------------------------------------------
// -----------------------------------------------------


//#define CF_MIN_RUNS 20
//#define CF_MAX_RUNS 40
#define CF_MAX_SEGS 40


typedef struct _Fault {
	int type;
	int level;  // new
	int NumSegs;
	FaultSeg fsegs[CF_MAX_SEGS];
} Fault;

/*
class Fault {
	int type;
	int level;  // new
	int NumSegs;
	FaultSeg fsegs[CF_MAX_SEGS];
public:
	static int decodeFaultFromName(char * fname,  int noerror = 0);
	static char * decodeFaultFromType(int ftype);

	static int decodeActionFromName(char * fname);
	static char * decodeActionFromType(int ftype);
	
	Fault(char *);
	int decodeType(char *);
	int decodeLevel(char *);
	int decodeNumSegs(char * a);
	// ------------
	char * getFaultName();
	int getFaultType();
	int getLevel(); // new
	int getNumSegs();
	// ------------
	int doFault();
	int undoFault();
	// ------------
	FaultSeg & operator[](int);
};
*/


static int Fault_decodeFaultFromName(char * fname, int noerror) { // noerror has default value 0
	if (!stricmp(FN_MFC,  fname)) return FT_MFC;           // outra non member ?
	if (!stricmp(FN_MLAC, fname)) return FT_MLAC;          // default = 0 - outra porra para converter
	if (!stricmp(FN_MVIV, fname)) return FT_MVIV;
	if (!stricmp(FN_MVIE, fname)) return FT_MVIE;
	if (!stricmp(FN_MIFS, fname)) return FT_MIFS;
	if (!stricmp(FN_MIA,  fname)) return FT_MIA;
	if (!stricmp(FN_MVAV, fname)) return FT_MVAV;
	if (!stricmp(FN_MVAE, fname)) return FT_MVAE;
	if (!stricmp(FN_MLPA, fname)) return FT_MLPA;
	// etc
	if (!stricmp(FN_WVAV, fname)) return FT_WVAV;
	if (!stricmp(FN_WLEC, fname)) return FT_WLEC;
	if (!stricmp(FN_WAEP, fname)) return FT_WAEP;
	if (!stricmp(FN_WPFV, fname)) return FT_WPFV;
	// etc
	if (noerror==0)
		printE("static Fault::decodeFaultFromName - unkown fault name");
	return -1;
}

static char * Fault_decodeFaultFromType(int ftype) {  // passou a non member
	switch (ftype) {
		case FT_MFC:  return FN_MFC;
		case FT_MLAC: return FN_MLAC;
		case FT_MVIV: return FN_MVIV;
		case FT_MVIE: return FN_MVIE;
		case FT_MIFS: return FN_MIFS;
		case FT_MIA:  return FN_MIA;
		case FT_MVAV: return FN_MVAV;
		case FT_MVAE: return FN_MVAE;
		case FT_MLPA: return FN_MLPA;
		// etc
		case FT_WVAV: return FN_WVAV;
		case FT_WLEC: return FN_WLEC;
		case FT_WAEP: return FN_WAEP;
		case FT_WPFV: return FN_WPFV;
		// etc
	}
	printE("static Fault::decodeFaultFromType - unkown fault type");
	return NULL;
}

static int Fault_decodeType(char * a) {
	static char temp[15];   // isto e static por que 
	char * b = strchr(a,',');
	int n = b-a;
	int ftype;
	if (n>14)
		printE("Fault::decodeType - type name too big - increase size of temp");
	strncpy(temp,a,n);
	temp[n] = '\0';
	ftype = Fault_decodeFaultFromName(temp,0); // o "0" era um param com def val = 0
	if (ftype == -1)
		printE("Fault::decodeType - unknown fault type");
	return ftype;
}

static int Fault_decodeLevel(char * a) {  // passou a non member
	int res = 0;
	char * b = strchr(a,',');
	if (!b)
		printE("Fault::decodeLevel - level terminator not found");
	while(a < b) {
		res = res * 10 + (*a - '0');
		a++;
	}
	if (res>100)
		printE("Fault::decodeLevel - level greater than 100 - is this right ?");
	return res;
}

static int Fault_decodeNumSegs(char * a) {  // passou a non member
	int res = 0;
	char * b = strchr(a,',');
	if (!b)
		printE("Fault::decodeNumSegs - num segs terminator not found");
	while(a < b) {
		res = res * 10 + (*a - '0');
		a++;
	}
	if (res==0)
		printE("Fault::decodeNumSegs - num segs is zero - this cannot be");
	return res;
}

static char * Fault_getFaultName(Fault * this) {
	return Fault_decodeFaultFromType(this->type);
}

static int Fault_getFaultType(Fault * this) { return this->type; }

static int Fault_getLevel(Fault * this) { return this->level; }

static int Fault_doFault(Fault * this) {
	// ***************************************
//	LPVOID base = (LPVOID) 0x77e61000;
	int i;
	for (i=0;i<this->NumSegs; i++)
		if (!FaultSeg_doFaultSeg(& this->fsegs[i])) {
			// printE("** Fault::doFault: unable to do a segment");  ja esta em FaultSeg
			// return 0;
			// que sa f continua para o proximo seg
			; // yeah - do nothing
		}
	return 1;
}


static int Fault_undoFault(Fault * this) {
//	LPVOID base = (LPVOID) 0x77e61000;
	int i;
	for (i=0;i<this->NumSegs; i++)
		if(!FaultSeg_undoFaultSeg(& this->fsegs[i]))  // estava "do" em vez de "undo"
			// printE("** Fault::undoFault: unable to undo a segment"); ja e impresso no FaultSeg
			// return 0;  que se lixe. continua
			;  // yeah - do nothing
	return 1;
}

static void Fault_Fault_default(Fault * this) {  // nao havia default ctor
	int i;
	for (i=0; i<CF_MAX_SEGS; i++)  // isto e' mesmo preciso para simular o defualt ctor de FaultSeg
		FaultSeg_FaultSeg_default(& (this->fsegs[i]) );   // que moca
	this->type = 0;  // ????  fara sentido?
	this->level = 0;  // new     ????
	this->NumSegs = 0;  // ??????? fara sentido?
}

static void Fault_Fault_withstring(Fault * this, char * fl) {
//	char temp[80];  // so para debug
	int i;
	Fault_Fault_default(this);  // para inicializar os FaultSegs	

//ok return;
	this->type = Fault_decodeType(fl);
	// --
	fl = strchr(fl,',')+1;
	this->level = Fault_decodeLevel(fl);
	// --
	fl = strchr(fl,',')+1;
	this->NumSegs = Fault_decodeNumSegs(fl);
	fl = strchr(fl,',')+1;
	if (this->NumSegs==0)
		printE("Fault::Fault(char*) - no segments in this fault - something is wrong");
	if (this->NumSegs>CF_MAX_SEGS)
		printE("Fault::Fault(char*) - too many segments in this fault - increase Fault::fsegs size");

// ok
// ok	sprintf(temp,"num segs=%d\n",this->NumSegs); print(temp);
// ok return;
	for (i=0; i< this->NumSegs; i++) {
		if (*fl=='*')
			printE("Fault::Fault(char*) - was expecting more segments in this fault");
		fl = FaultSeg_decodeFaultSeg(& this->fsegs[i],fl);
	}
	if (*fl != '*')
			printE("Fault::Fault(char*) - it seems to be more segs int this fault than expected");
}


static void Fault_print(Fault * this) {
	static char temp[80];
	print("K: ----------------------------------\n");
	sprintf(temp,"K: Type: %s\n",Fault_decodeFaultFromType(this->type)); print(temp);
	sprintf(temp,"K: Num Segments: %d\n", this->NumSegs); print(temp);
	print("K: Segment 1 is:\n");
	FaultSeg_print( & (this->fsegs[0]) );
	print("K: ----------------------------------\n");
}

// ------------------------------------------------------------------------------


// isto e so para testes carago
char mfc[] = "MFC,1,1,SUBS,c03ae798,5,e8d30e0100,9090909090,# call 0xc03bf670 #,*";

/*
int main() {
	Fault * f = (Fault *) malloc(sizeof(Fault));
	Fault_Fault_withstring(f, mfc);

	Fault_print(f);

	return 0;
}
*/


// ------------------------------------------------------------------------
// KPA stuff is above this point
// ------------------------------------------------------------------------








// whatever


// more whatever

#define IOBSIZE 8190
static char * inputb;  // input received from vfi
static char * outputb; // output to show to cat
static char * faultb;  // where fault "object" will be placed

  
//  Prototypes - this would normally go in a .h file
//  but i really really dont care

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "kfi"	// Dev name as it appears in /proc/devices   
//#define BUF_LEN 200		// Max length of the message from the device 

 
// Global variables are declared as static, so are global within the file. 


static int Major;		// Major number assigned to our device driver
static int Device_Open = 0;	// Is device open?  
				// Used to prevent multiple access to device
//static char msg[BUF_LEN];	// The msg the device will give when asked
//static char *msg_Ptr;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};


// This function is called when the module is loaded
// oh boy oh boy cant wait for that

int init_module(void) {
   char temp[240]; // sorry, kernel stack

   Major = register_chrdev(0, DEVICE_NAME, &fops);

   if (Major < 0) {
      sprintf(temp, KERN_ALERT "Registering char device failed with %d\n", Major);
      printk(temp); print(temp);
      return Major;
   }

   // specific stuff;
   inputb = vmalloc(IOBSIZE);
   if (inputb == NULL) {
      sprintf(temp, KERN_ALERT "input buffer allocation failed\n");
      printk(temp); print(temp);
      return -1;   // seja la' oque '-1' signifique
   }
   outputb = vmalloc(IOBSIZE);
   if (outputb == NULL) {
      sprintf(temp, KERN_ALERT "output buffer allocation failed\n");
      printk(temp); print(temp);
      vfree(inputb);
      return -1;
   }

   faultb = vmalloc(sizeof(Fault) + 10); // o 10 e' para dar sorte
   if (faultb == NULL) {
      sprintf(temp, KERN_ALERT "Fault object buffer allocation failed\n");
      printk(temp); print(temp);
      vfree(inputb);
      vfree(outputb);
      return -1;
   }

   *inputb = 0;
   strcpy(outputb,"OUTPUT BUFFER IS EMPTY");
	
   sprintf(temp, KERN_INFO "module was assigned major number %d.\n", Major);
   printk(temp); print(temp);
	
   sprintf(temp, KERN_INFO "dont forget: 'mknod device-name c %d 0'.\n", Major);
   printk(temp); print(temp);

   return SUCCESS;
}


// This function is called when the module is unloaded
// really? wow.....

void cleanup_module(void) {
   char temp[240];
   // Unregister the device 

   int ret = unregister_chrdev(Major, DEVICE_NAME);
   if (ret < 0) {
      sprintf(temp, KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
      printk(temp); print(temp);
   }

   if (inputb)  vfree(inputb);
   if (outputb) vfree(outputb);
   if (faultb)  vfree(faultb);
}


// Methods
// oi. this is not c++

 
// Called when a process tries to open the device file, like
// "cat /dev/mycharfile"

static int device_open(struct inode *inode, struct file *file) {
//   static int counter = 0;

   if (Device_Open)
      return -EBUSY;

   Device_Open++;
//   sprintf(msg, "I already told you %d times Hello world!\n", counter++);
//   msg_Ptr = msg;
   print("K: Got opened");
   
   try_module_get(THIS_MODULE);

   // inicio do kernel e c0100000
   // tamanho do kernel talvez seja c0437ce0
   // ret = c3

   return SUCCESS;
}

 
// Called when a process closes the device file.

static int device_release(struct inode *inode, struct file *file) {
   Device_Open--;		// We're now ready for our next caller //
	
   // Decrement the usage count, or else once you opened the file, you'll
   // never get get rid of the module. 
   
   print("K: Got closed");	
   module_put(THIS_MODULE);

   return 0;
}
 
// Called when a process, which already opened the dev file, attempts to
// read from it.
 
static ssize_t device_read(struct file *filp,	// see include/linux/fs.h   
			   char *buffer,	// buffer to fill with data 
			   size_t length,	// length of the buffer     
			   loff_t * offset) {   // que raio e este ultimo?

   // Number of bytes actually written to the buffer
   int bytes_read = 0;
   char * outptr = outputb;

   // some sanity check. after all this is ...... the kerne'u .... (brasillian accent)
   if (!outputb)
      return 0; // sorry no can do

   // If we're at the end of the message, 
   // return 0 signifying end of file 

   if (*outptr == 0)
      return 0;
	 
   // Actually put some crap into the buffer (int user mode space)
	
   while (length && *outptr && (bytes_read < IOBSIZE) ) { // raios. estava msg_Ptr
      // bugs no kerneu é fuuuudido                          em vez de outptr
      // The buffer is in the user data segment, not the kernel 
      // segment so "*" assignment won't work.  We have to use 
      // put_user which copies data from the kernel data segment to
      // the user data segment. 

      put_user(*(outptr++), buffer++);

      length--;
      bytes_read++;

      // um byte de cada vez? porra, deve ser lento como o raio
   }

   *outputb = 0;   // next time dont read this again (will take a EOF)
   // Most read functions return the number of bytes put into the buffer
	
   return bytes_read;
}

  
// Called when a process writes to dev file: echo "hi" > /dev/hello 

static ssize_t device_write(struct file *filp, const char *buffer,
                            size_t length, loff_t * off) {
	
   char temp[240];   // tinha um * - ja comeca a ter bugs (compile time warning)
   int bytes_written = 0;
   char * inptr = inputb;
  
   Fault * fptr; // so ponteiro - ele vai ser posto em cima da iobuffer !!!!! faultb carago
  	
   // some sanity
   if (!inputb)
      return 0;   // sorry, no can do

   while (length && (bytes_written<IOBSIZE) ) {   // && *buffer saiu (era nosense)
      // bla bla user data segment => get_user      no can do *buffer
      get_user(*(inptr++), buffer++);  // espero que os parametros sejam estes
      length--;
      bytes_written++;
      //idem um byte de cada vez
      // se for so texto mesmo havia de ter um teste qualquer de \0
   }
   if (bytes_written < IOBSIZE)
      *inptr = 0;      // convem que isto vai ser tratado como texto

//   print("-----------------------------------------------------\n");
   sprintf(temp, "K: Got written %d bytes\n", bytes_written);
   print(temp);

   // faz qualquer coisa com a porra que lhe foi escrita
   strcpy(outputb,"K: Received[");   // good for you
   strcat(outputb,inputb);
   strcat(outputb,"]\n");
//   print("-----------------------------------------------------");
   fptr = (Fault *) faultb;                  // see *faultb as a Fault. These are not the robots you are looking for
   Fault_Fault_withstring(fptr, inputb+1);   // first char is +/-
// ok so far????

   // iobuffer is ok.  i saw that with a print
   // print("\n\n\n\n\n\n\n\n\n\n\n\n\n");
   // print(inputb);
   Fault_print(fptr);

// o seguinte e so para memorias de bugs   
//   Fault_Fault_withstring(fptr, inputb+1);  // que loucura: estava a criar o 
//   Fault_print(fptr);                       // objecto "Fault" em cima da informacao
//   print("--------");                       // necessaria a sua info. need a 3rd buffer

   // here goes the code to f*ck/unf*ck the kernel

   if (*inputb == '+') {
      print("DOING fault NOW\n");
      Fault_doFault(fptr);
   } else if (*inputb == '-') {
      print("UNDOING fault NOW\n");
      Fault_undoFault(fptr);
   } else
      print("Dont know what to do with fault. Use only '+' and '-' when talking to me\n");
   
   print("K: done receiving fault and do/undo ing it\n"); 
   return bytes_written;

   // claro que e', carago
   // printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
   // return -EINVAL;
}
