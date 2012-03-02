/**
 
	jd.ringave - running average v1.0 
    2.2012
    joshua dickinson - joshuadaviddickinson@gmail	
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

////////////////////////// object struct
typedef struct _ringave 
{
	t_object					ob;			// the object itself (must be first)
    //long s_value; // something else
    long mod;
    long counter;
    float data[1000];
    void *m_outlet1;
    void *m_outlet2;
    
} t_ringave;

///////////////////////// function prototypes
//// standard set
void *ringave_new(t_symbol *s, long argc, t_atom *argv);
void ringave_free(t_ringave *x);
void ringave_assist(t_ringave *x, void *b, long m, long a, char *s);
void simp_bang(t_ringave *x);
void simp_int(t_ringave *x, long n);
void simp_float(t_ringave *x, double n);
void myobject_in1(t_ringave *x, long n);//inlet 1

//////////////////////// global class pointer variable
void *ringave_class;


int main(void)
{	
	// object initialization, NEW STYLE
	t_class *c;
	
	c = class_new("jd.ringave", (method)ringave_new, (method)ringave_free, (long)sizeof(t_ringave), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)ringave_assist,			"assist",		A_CANT, 0);  
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    
    class_addmethod(c, (method)simp_int, "int", A_LONG, 0);
    class_addmethod(c, (method)simp_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)simp_bang, "bang", 0);
    class_addmethod(c, (method)myobject_in1, "in1", A_LONG, 0);
    class_register(CLASS_BOX, c);
    
    ringave_class = c;

	post("jd.ringave v1.0 - Josh Dickinson - 2012");
	return 0;
    
}

void ringave_assist(t_ringave *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
		//sprintf(s, "I am inlet %ld", a);
        sprintf(s, "I am inlet %ld", a);
	} 
    
	else {	// outlet
		sprintf(s, "Average"); 			
	}
}

void ringave_free(t_ringave *x)
{
	;
}

void *ringave_new(t_symbol *s, long argc, t_atom *argv)
{
	t_ringave *x = NULL;
    long i;

    
	// object instantiation, NEW STYLE
	if (x = (t_ringave *)object_alloc(ringave_class)) {
        //object_post((t_object *)x, "a new %s object was instantiated: 0x%X", s->s_name, x);
        //object_post((t_object *)x, "it has %ld argu", argc);
        intin(x, 1);//inlet 1
        //x->m_outlet2 = bangout((t_object *)x);
        x->m_outlet1 = floatout((t_object *)x);
        
        
        
        x->counter = 0;
        x->mod = 1;        
        
        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                long tempArg = atom_getlong(argv+i);
                object_post((t_object *)x, "arg %ld: long (%ld)", i, tempArg);
                if(i==0 && tempArg > 0 && tempArg <= 1000){
                    post("initial buffer size set to %ld", tempArg);
                    x->mod = atom_getlong(argv+i);
                }
                
            } else if ((argv + i)->a_type == A_FLOAT) {
                object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
            } else {
                object_error((t_object *)x, "forbidden argument");
            }
        }
	}

    
	return (x);
}

void simp_bang(t_ringave *x)
{
    
}

void simp_int(t_ringave *x, long n)
{
    x->data[x->counter] = n;
    x->counter = (x->counter + 1) %x->mod;
    float temp = 0;
    int i;
    for(i = 0;i<(x->mod);i++){
        temp += x->data[i];
    }
    temp /= x->mod;
    //post("Your average over %ld frames is: %f",x->mod, temp);
    outlet_float(x->m_outlet1, temp);
}

void simp_float(t_ringave *x, double n)//you have to use DOUBLE, not float
{
    post("test float %f", n);
    x->data[x->counter] = n;
    x->counter = (x->counter + 1) %x->mod;
    float temp = 0;
    int i;
    for(i = 0;i<(x->mod);i++){
        temp += x->data[i];
    }
    temp /= x->mod;
    //post("Your average over %ld frames is: %f",x->mod, temp);
    outlet_float(x->m_outlet1, temp);
}

void myobject_in1(t_ringave *x, long n)
{
    if(n<1 || n>1000){
        post("sorry friend, the buffer size can't be less than 1 or more than 1000");
        return;
    }
    post("new mod: %ld",n);
    x->counter = 0;
    x->mod = n;
}
