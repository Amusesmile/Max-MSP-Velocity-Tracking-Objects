/**
 
	jd.derivation - running difference v1.0 
    2.2012
    joshua dickinson - joshuadaviddickinson@gmail	
*/

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object

////////////////////////// object struct
typedef struct _derivation 
{
	t_object ob;			// the object itself (must be first)
    long mod, counter;
    float prevPos, prevVel, prevAcc;
    float velThresh, accThresh, jerkThresh;
    float vel[1000], acc[1000], jerk[1000];
    bool velSwitch, accSwitch, jerkSwitch;
    void *m_out1, *m_out2, *m_out3, *m_out4, *m_out5, *m_out6; 

} t_derivation;

///////////////////////// function prototypes
//// standard set
void *derivation_new(t_symbol *s, long argc, t_atom *argv);
void derivation_free(t_derivation *x);
void derivation_assist(t_derivation *x, void *b, long m, long a, char *s);
void simp_bang(t_derivation *x);
void simp_int(t_derivation *x, long n);
void simp_float(t_derivation *x, double n);
void calculateValues(t_derivation *x, double n);
void myobject_in1(t_derivation *x, long n);//inlet 1
void myobject_in2(t_derivation *x, double n);//inlet 2
void myobject_in3(t_derivation *x, double n);//inlet 3
void myobject_in4(t_derivation *x, double n);//inlet 4


//////////////////////// global class pointer variable
void *derivation_class;


int main(void)
{	
	// object initialization, NEW STYLE
	t_class *c;
	
	c = class_new("jd.derivation", (method)derivation_new, (method)derivation_free, (long)sizeof(t_derivation), 
				  0L /* leave NULL!! */, A_GIMME, 0);
	
	/* you CAN'T call this from the patcher */
    class_addmethod(c, (method)derivation_assist,			"assist",		A_CANT, 0);  
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
    
    class_addmethod(c, (method)simp_int, "int", A_LONG, 0);
    class_addmethod(c, (method)simp_float, "float", A_FLOAT, 0);
    class_addmethod(c, (method)simp_bang, "bang", 0);
    class_addmethod(c, (method)calculateValues, "float", A_FLOAT, 0);
    class_addmethod(c, (method)myobject_in1, "in1", A_LONG, 0);
    class_addmethod(c, (method)myobject_in2, "in2", A_FLOAT, 0);
    class_addmethod(c, (method)myobject_in3, "in3", A_FLOAT, 0);
    class_addmethod(c, (method)myobject_in4, "in4", A_FLOAT, 0);
    class_register(CLASS_BOX, c);
    
    derivation_class = c;

	post("jd.derivation v1.0 - Josh Dickinson - 2012");
	return 0;
    
}

void derivation_assist(t_derivation *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET) { // inlet
        //sprintf(s, "I am inlet %ld", a);
        if(a == 4){
            sprintf(s, "buffer size");
        }
        if(a == 3){
            sprintf(s, "jerk bang threshold");
        }
        if(a == 2){
            sprintf(s, "accelleration bang threshold");
        }
        if(a == 1){
            sprintf(s, "velocity bang threshold");
        }
        if(a == 0){
            sprintf(s, "one-dimensional position values");
        }        
	} 
    
	else {	// outlet
        if(a == 5){
            sprintf(s, "jerk threshold bang");
        }
        if(a == 4){
            sprintf(s, "moving average jerk");
        }
        if(a == 3){
            sprintf(s, "acceleration threshold bang");
        }
        if(a == 2){
            sprintf(s, "moving average acceleration");
        }
        if(a == 1){
            sprintf(s, "velocity threshold bang");
        }   	
        if(a == 0){
            sprintf(s, "moving average velocity");
        }
	}
}

void derivation_free(t_derivation *x)
{
	;
}

void *derivation_new(t_symbol *s, long argc, t_atom *argv)
{
	t_derivation *x = NULL;
    long i;
    
	// object instantiation, NEW STYLE
	if (x = (t_derivation *)object_alloc(derivation_class)) {
        //object_post((t_object *)x, "a new %s object was instantiated: 0x%X", s->s_name, x);
        //object_post((t_object *)x, "it has %ld argu", argc);
        
        intin(x, 1);//inlet 1
        intin(x, 4);//inlet 1
        intin(x, 3);//inlet 1
        intin(x, 2);//inlet 1        
        
        x->counter = 0;
        x->mod = 1;
        x->prevPos = 0.0;
        x->prevVel = 0.0;
        x->prevAcc = 0.0;
        x->velThresh = 1.0;
        x->accThresh = 1.0;
        x->jerkThresh = 1.0;
        
        x->m_out6 = bangout((t_object *)x);
        x->m_out5 = floatout((t_object *)x);
        x->m_out4 = bangout((t_object *)x);
        x->m_out3 = floatout((t_object *)x);
        x->m_out2 = bangout((t_object *)x);
        x->m_out1 = floatout((t_object *)x);        
        
        for (i = 0; i < argc; i++) {
            if ((argv + i)->a_type == A_LONG) {
                long tempArg = atom_getlong(argv+i);
                //object_post((t_object *)x, "arg %ld: long (%ld)", i, tempArg);
                if(i==0 && tempArg > 0 && tempArg <= 1000){
                    post("initial buffer size set to %ld", tempArg);
                    x->mod = tempArg;
                }              
                
            } else if ((argv + i)->a_type == A_FLOAT) {
                //object_post((t_object *)x, "arg %ld: float (%f)", i, atom_getfloat(argv+i));
                float tempArg = atom_getfloat(argv+i);
                if(i==1 && tempArg > 0 && tempArg <= 1000){
                    post("initial velocity bang threshold %f", tempArg);
                    x->velThresh = tempArg;
                }  
                if(i==2 && tempArg > 0 && tempArg <= 1000){
                    post("initial accleration bang threshold %f", tempArg);
                    x->accThresh = tempArg;
                }  
                if(i==3 && tempArg > 0 && tempArg <= 1000){
                    post("initial jerk bang threshold %f", tempArg);
                    x->jerkThresh = tempArg;
                }  
            } else if ((argv + i)->a_type == A_SYM) {
                object_post((t_object *)x, "arg %ld: symbol (%s)", i, atom_getsym(argv+i)->s_name);
            } else {
                object_error((t_object *)x, "forbidden argument");
            }
        }
	}    
	return (x);
}

void simp_bang(t_derivation *x)
{
    
}

void simp_int(t_derivation *x, long n)
{
    double nd = (double)n;
    calculateValues(x,nd);
    
    /*x->vel[x->counter] = n-(x->prevPos);
    x->counter = (x->counter + 1) %x->mod;
    float temp = 0;
    int i;
    for(i = 0;i<(x->mod);i++){
        temp += x->vel[i];
    }
    temp /= x->mod;
    outlet_float(x->m_out1, temp);
    x->prevPos = n;*/
}

void simp_float(t_derivation *x, double n)//you have to use DOUBLE, not float
{
    calculateValues(x,n);
}

void calculateValues(t_derivation *x, double n){
    
    //vel
    x->vel[x->counter] = n-(x->prevPos);
    float tempVel = 0;
    int i;
    for(i = 0;i<(x->mod);i++){
        tempVel += x->vel[i];
    }
    tempVel /= x->mod;
    outlet_float(x->m_out1, tempVel);
    if(abs(tempVel) > x->velThresh && x->velSwitch == false){
        x->velSwitch = true;
        outlet_bang(x->m_out2);
        //post("vel thresh reached");
    }
    if(abs(tempVel) < x->velThresh){
        x->velSwitch = false;
        //post("below thresh");
    }
    x->prevPos = n;
    
    
    //accel
    x->acc[x->counter] = tempVel-(x->prevVel);
    float tempAcc = 0;
    for(i = 0;i<(x->mod);i++){
        tempAcc += x->acc[i];
    }
    tempAcc /= x->mod;
    outlet_float(x->m_out3, tempAcc);
    if(abs(tempAcc) > x->accThresh && x->accSwitch == false){
        x->accSwitch = true;
        outlet_bang(x->m_out4);
    }
    if(abs(tempAcc) < x->accThresh){
        x->accSwitch = false;
    }
    x->prevVel = tempVel;
    
    
    //jerk
    x->jerk[x->counter] = tempAcc-(x->prevAcc);
    float tempJerk = 0;
    for(i = 0;i<(x->mod);i++){
        tempJerk += x->jerk[i];
    }
    tempJerk /= x->mod;
    outlet_float(x->m_out5, tempJerk);
    if(abs(tempJerk) > x->jerkThresh && x->jerkSwitch == false){
        x->jerkSwitch = true;
        outlet_bang(x->m_out6);
    }
    if(abs(tempJerk) < x->jerkThresh){
        x->jerkSwitch = false;
    }
    x->prevAcc = tempAcc;    
    
    //post("vel: %f   acc: %f jerk: %f",tempVel, tempAcc, tempJerk);
    x->counter = (x->counter + 1) %x->mod;
}

void myobject_in1(t_derivation *x, long n)
{
    if(n<1 || n>1000){
        post("buffer size must be 1-1000");
        return;
    }
    post("new buffer size: %ld",n);
    x->counter = 0;
    x->mod = n;
}

void myobject_in2(t_derivation *x, double n)
{
    n=abs(n);
    x->velThresh = n;
    post("new velocity bang threshold: %f",n);
}

void myobject_in3(t_derivation *x, double n)
{
    n=abs(n);
    x->accThresh = n;
    post("new acceleration bang threshold: %f",n);
}

void myobject_in4(t_derivation *x, double n)
{
    n=abs(n);
    x->jerkThresh = n;
    post("new jerk bang threshold: %f",n);
}

