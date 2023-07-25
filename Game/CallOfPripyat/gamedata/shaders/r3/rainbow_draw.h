#define RB_INTENSITY float(0.2)			// Absolute maximal intensitiy of the rainbows.
#define RB_SECONDARY float(0.1)			// Relative intensitiy of the secondary rainbow.
#define RB_SATURATION float(0.75)		// Saturation of the rainbows.
#define RB_DECAY float(5.0)				// Intensity decay / falloff.
#define RB_COLRATIO float(0.60)			// Color ratio.
#define RB_COLSHIFT float(0.0)			// Color shift.
#define RB_COLPOWER float(0.52)			// Color power.	
#define RB_ENDXMIN float(-6.0)			// AMOUNT OF FADE OUT TOWARDS THE GROUND. Best between -6 and 5.
#define RB_ENDXMAX float(5.0)			// AMOUNT OF FADE OUT TOWARDS THE GROUND. Best between -6 and 5.
#define RB_COFX float(1.5)				// AMOUNT OF COLOR TOWARDS THE GROUND. Best between 1.5 and 2.
#define RB_ENDY float(2.0)				// AMOUNT OF FADE OUT TOWARDS THE SKY
#define RB_COFY float(1.0)				// AMOUNT OF COLOR TOWARDS THE SKY
#define RB_VIEWANGLE float4(0.5,1.0,-1.0,0.999)	// Minimum and maximum view angle for rainbows to show up. Required to avoid false positives. Do not change normally.
#define RB_GREENREDDIFF float(-0.05)	// Minimum green-red color difference for rainbows to show up. To be adjusted according to used weather mod. Default weather: rainbow at around 7AM, 11AM, 2PM, 8PM.
#define RB_GREENBLUEDIFF float(0.05)	// Minimum green-blue color difference for rainbows to show up. To be adjusted according to used weather mod.	
#define RB_POSITION float2(0.9,-0.6)	// Rainbow x/y position. Must be fix. Do not change normally. X-coord also be 0.8.	
#define RB_ORIGIN float2(-2.0,-0.6)		// Coordinate system origin for computing rainbow. Do not change unless changing RB_POSITION.
#define RB_COF float(9.0)				// Maximum intensity of rainbows.
#define RB_SUNFACTOR float(3.0)			// Effect amount of sunlight intensity on rainbow shape/length.
#define RB_WIDESCREEN					// Enables wide-screen bow adjustment. Disable if not using a wide-screen monitor.	
#define RB_DEBUGPOS					// Debug only.



float4 ___z523(float f,bool R)
{
	f+=.333;f=-(f-.5)+.5;

	float i=(int)(f*6%6)+5*(f<0);
	float p=f*6%1+(f<0);
	float s=1-p,b,z,T;
	if(i==0)b=1,z=p,T=0;
	else if(i==1)b=s,z=1,T=0;
	else if(i==2)b=0,z=1,T=p;
	else if(i==3)b=0,z=s,T=1;
	else if(i==4)b=p,z=0,T=1;
	else if(i==5)b=1,z=0,T=s;
	if(R)return float4(1,1,1,1);
	else return float4(b,z,T,1);
}
float4 draw_rainbow(float2 s,float2 R,float i,bool f,bool T)
{
	float4 b=float4(0,0,0,0);
	float z=.16,p=.1;float2 x=1.3,X=x+z,Y=1.6,r=Y+p;
	if(T)z=.18,x=.9,p=.75,Y=1.2,R.y+=.2;float2 N=s;
#ifdef RB_WIDESCREEN
N.y*=10/16.f;
#endif
float O=distance(R,N),o=(N.x-R.x)/x,a=(N.y-R.y)/x,t=(N.x-R.x)/Y,l=(N.y-R.y)/Y;
if(f&&N.x<R.x)
{
	float y=0,e=0;float4 d=float4(0,0,0,0);
	float B=abs(RB_ENDXMAX-RB_ENDXMIN),E=RB_ENDXMIN+i*B;
	if(O>=x.x&&O<=X.x){y=(O-x)/(X-x);
	d=___z523(pow(y*RB_COLRATIO-RB_COLSHIFT,RB_COLPOWER),T);
	e=1*pow(1-abs(y-.5),RB_DECAY);
	float C=saturate(RB_COFX-pow(o,E)),w=saturate(RB_COFY-pow(a,RB_ENDY));
	e*=C*w;}else if(O>=Y.x&&O<=r.x){y=(O-Y)/(r-Y);
	d=___z523(1-pow(y*RB_COLRATIO-RB_COLSHIFT,RB_COLPOWER),T);
	e=RB_SECONDARY*pow(1-abs(y-.5),RB_DECAY);
	float C=saturate(RB_COFX-pow(t,E)),w=saturate(RB_COFY-pow(l,RB_ENDY));
	e*=C*w;}float C=dot(d.xyz,float3(.3,.59,.11));
	d.xyz=lerp(float3(C,C,C),d.xyz,RB_SATURATION);
	b=(1-RB_INTENSITY*e)*b+RB_INTENSITY*e*d;
	}
	return b;
}