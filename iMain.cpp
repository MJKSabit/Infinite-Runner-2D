#define _CRT_SECURE_NO_WARNINGS
# include "iGraphics.h"

int ignoreColor = 0x804080;
char *background = "bg.bmp";


const int height = 600, width = 1000;


// char **imgUnion[3];

const int velocityTimeStep = 21, heightY = 20, initPosY = 40;
int velT;


int soilW = 37, posSoil = soilW;
int bgW = 200, bgPos = bgW, bg2Pos=bgW;
int cloudPos[3], cloudX[3], cloudMin = 300, cloudMax = 550;

//int i;

/// Color Modules
typedef struct
{
    int r, g, b;
} Color;
void iSetColor(Color color)
{
    iSetColor(color.r, color.g, color.b);
}
Color fColor(int r, int g, int b)
{
    Color temp = {r, g, b};
    return temp;
}
/// Color END


/// Image Module
typedef struct
{
    double posX, posY;//, velY, decY;
    //double centerX, centerY, yRange, xRange;

    //int idx[3], state; /// state = 0: running, 2: jumping, 1: throwing
    char location[40];
} Image;

Image fImage(char* loc, int posX, int posY)
{
    Image res;
    strcpy(res.location, loc);
    res.posX = posX;
    res.posY = posY;

    return res;
}

/// Image END

/// Objects
enum ObjectType {IMAGE, REPEATED_IMAGE, RECTANGLE};

typedef struct
{
    ObjectType kind;
    double centerX, centerY;
    double height, width; /// Not actual Height Width
    double velocityX, velocityY;

    clock_t StartTime;

    Image img; ///In case of image

} Object;

Object fObject(ObjectType t, double posX, double posY, double h, double w)
{
    Object res;
    res.kind = t;
    res.height = h/2;
    res.width = w/2;
    res.centerX = posX;
    res.centerY = posY;
    //res.StartTime = clock();

    return res;
}

double getCenterY(Object ob)
{
    return ob.centerY+ob.height;
}
double getCenterX(Object ob)
{
    return ob.centerX+ob.width;
}

void resetClock(Object *obj)
{
    obj->StartTime = clock();
}
void drawImage(Object img);
/// Objects END

void animateRunner();

Object runner;
int runnerState, runningIndex, throwingIndex=-1, jumpingIndex;
char *run[6] = {"r1.bmp", "r2.bmp", "r3.bmp", "r4.bmp", "r5.bmp", "r6.bmp"};
char *thr[3] = {"t1.bmp", "t2.bmp", "t3.bmp"};
char *jump[4] = {/*"j1.bmp",*/ "j2.bmp", "j3.bmp", "j4.bmp", "j5.bmp"};
char *arrow_loc = "arrow.bmp";
int numberOfPic[] = {6, 3, 4};

clock_t animatingTime = 50, last_cloud = -4000, last_obstacle; /// In MiliSec

const int jumpingHeight = 120, jumpingTStep=17; /// In Pixel
int jumpingT=-1;
double jumpingVel;// = 4.0*jumpingHeight/jumpingTime;
double jumpingAcc;
Color ObsCol1, ObsCol2;

Object soil, bg1, bg2, obstacle[4];

void animateRunning()
{
    runningIndex++;
    if (runningIndex >= numberOfPic[runnerState])
        runningIndex -= numberOfPic[runnerState];

    strcpy(runner.img.location, run[runningIndex]);
}


typedef int Arrow;
int numOfArrows = 15;
Arrow arrowPos[15];

void throwArrow()
{
    int j;
    for (j = numOfArrows-1; j > 0; j--)
        arrowPos[j] = arrowPos[j-1];
    arrowPos[0] = runner.centerX + 2*runner.width; /// From Runners End
}

void animateThrowing()
{
    throwingIndex++;

    if(throwingIndex == numberOfPic[runnerState])
    {
        runnerState = 0; /// RunningState
        throwingIndex = -1;

        throwArrow();
        animateRunner();
    }
    else
    {
        strcpy(runner.img.location, thr[throwingIndex]);
    }
}

void animateJumping()
{
    jumpingT++;


    if(jumpingT == 0)   /// Start of a new jump
    {
        resetClock(&runner);
        runner.velocityY = jumpingVel;
        jumpingAcc = jumpingVel*2.0/jumpingTStep;
        runningIndex = 0;
    }

    //printf("%d %f %f %f\n", jumpingIndex, jumpingVel, jumpingAcc, getPosY(runner));

    clock_t time = clock()-runner.StartTime;

    if((time) > animatingTime*jumpingTStep || runner.centerY < initPosY)  /// End of Jumping
    {
        jumpingIndex = -1;
        runnerState = 0;
        runner.centerY = initPosY;
        runningIndex = 4;
        jumpingT=-1;
        animateRunner();
    }
    else  /// Continue jumping Until time comes
    {
        double timeRatio = double(jumpingT)/jumpingTStep;

        if(timeRatio<0.1)
            jumpingIndex = 0;
        else if(timeRatio<0.5)
            jumpingIndex = 1;
        else if(jumpingIndex<0.9)
            jumpingIndex = 2;
        else
            jumpingIndex = 3;

        strcpy(runner.img.location, jump[jumpingIndex]);

        runner.velocityY -= jumpingAcc;
        runner.centerY += runner.velocityY;
        //setPosY(&runner, getPosY(runner)+runner.velocityY);

    }

}


/** todo: use enum instead of 0, 1 for state */
void animateRunner()
{

    if(runnerState==0)
    {
        animateRunning();
    }
    else if(runnerState==1)
    {
        animateThrowing();
    }
    else
    {
        animateJumping();
    }
}

void moveObject( Object *obj, double dx, double dy=0)
{
    obj->centerX -= dx;
    if(obj->centerX<0)
    {
        if(obj->kind==REPEATED_IMAGE)
            obj->centerX += 2*obj->width;
        else ;/// Delete object
    }

    obj->centerY -= dy;
    if(obj->centerY<0)
    {
        if(obj->kind==REPEATED_IMAGE)
            obj->centerY += 2*obj->height;
        else ;/// Delete object
    }

}

void moveCloud()
{
    int i;
    for(i=0; i<3; i++)
        if(cloudX[i]<=0)  /// Assign Random Cloud
        {
            cloudPos[i] = (rand()+clock()%1000)%cloudMax;

            if(cloudPos[i]>cloudMin && clock()-last_cloud>5000+rand()%4000)
            {
                last_cloud = clock();
                cloudX[i] = width;
            }
        }

    for(i=0; i<3; i++)
    {
        cloudX[i]-=3;
        if(cloudX[i]<=0)
            cloudPos[i] = 0;
    }
}

void moveObstacles(int px)
{
    int i;
    for(i=0; i<4; i++)
    {
        if(obstacle[i].centerX>0)
            moveObject(&obstacle[i], px); /// Obstacles Movement
        else
        {
            if(clock()-last_obstacle>1500+(rand()%100+1)*(rand()%10+10) && clock()%3==0)  /// Obstacle Timer
            {
                last_obstacle = clock();
                obstacle[i].centerX = width;
            }
        }
    }
}

void checkRunnerObstacleCollusion();

void animate()
{
    moveObject(&soil, 15);
    moveObstacles(15);
    moveObject(&bg1, 10);
    moveObject(&bg2, 5);
    moveCloud();

    animateRunner();
    // checkRunnerObstacleCollusion();
}


void drawArrow()
{
    int i;
    for(i=0; i<numOfArrows; i++)
    {
        if(arrowPos[i]>width || arrowPos[i]<=0)
            arrowPos[i] = 0;
        else
        {
            arrowPos[i]+=5;
            iShowBMP2(arrowPos[i], 70, arrow_loc, ignoreColor);
        }
        //printf("%d %3d\n", i, arrowPos[i]);
    }
}

void drawCloud()
{
    int i;
    for(i=0; i<3; i++)
    {
        if(cloudX[i]>0 && cloudPos[i]>cloudMin)
            iShowBMP2(cloudX[i], cloudPos[i], "cloud.bmp", ignoreColor);
    }
}

void drawImage(Object img)
{
    if(img.kind==IMAGE)
        iShowBMP2(img.centerX, img.centerY, img.img.location, ignoreColor);
    else if(img.kind==REPEATED_IMAGE)
    {
        int i;
        for(i=0; 2*i*img.width<width; i++)
            iShowBMP2(img.centerX+2*i*img.width, img.centerY, img.img.location, ignoreColor);
    }
    else
    {
        iFilledRectangle(img.centerX, img.centerY, img.width, img.height);
    }
}

void checkArrowObstacleCollusion()
{
    int i, j;

    for(i=0; i<numOfArrows; i++)
    {
        for(j=0; j<4 && arrowPos[i]>0; j++)
        {

            if(obstacle[j].centerX-arrowPos[i]<=obstacle[j].width && 2*obstacle[j].centerX>width && arrowPos[i]>5)
            {
                obstacle[j].centerX = 0;
                arrowPos[i] = 0;
                // printf("%d %d\n", i, j);
                break;
            }
        }
    }
}

int checkPoint(double x, double y, Color CheckColor)
{
    int col[3];
    iGetPixelColor(x, y, col);

    return (col[0]==CheckColor.r && col[1]==CheckColor.g && col[2]==CheckColor.b);
}

void checkRunnerObstacleCollusion() /// Change of Algorithm: Check border for other color :: DONE
{
    int i;
    for(i=0; i<4; i++)
    {
        if(abs(obstacle[i].centerX-runner.centerX) <= (runner.width+obstacle[i].width) && obstacle[i].centerX>200)
        {
            // printf("%f %f %f %f\n", obstacle[i].centerY, obstacle[i].height, runner.centerY, runner.height);

            int nowX = obstacle[i].centerX+4, nowXmax, nowY = obstacle[i].centerY+4, nowYmax;
            nowXmax = nowX + obstacle[i].width-8;
            nowYmax = nowY + obstacle[i].height-8;

            int tempX, tempX2, tempY, flag=1;

            for(tempX=nowX, tempX2=nowXmax, tempY=nowY; tempY<=nowYmax && flag && (runnerState==0 || runnerState==1); tempY+=10){
                flag = checkPoint(tempX, tempY, ObsCol1) && checkPoint(tempX2, tempY, ObsCol1);
            }

            for(tempX=nowX, tempY=nowYmax; tempX<=nowXmax && flag && runnerState==2; tempX+=7){
                flag = checkPoint(tempX, tempY, ObsCol1);
            }

            if(!flag) iPauseTimer(0);
        }
    }
}

void drawObstacle()
{
    int i;
    for(i=0; i<4; i++)
    {
        if(obstacle[i].centerX>0)
            drawImage(obstacle[i]);
    }

}

/// Called Every time
void iDraw()
{
    int i;
    iClear();
    iSetColor(fColor(18, 39, 60));
    iFilledRectangle(0, 0, width, height);
    drawCloud();
    //drawSoil();
    iSetColor(ObsCol1);
    //drawObs();
    drawImage(bg2);
    drawImage(bg1);
    drawImage(soil);
    drawObstacle();
    drawImage(runner);
    drawArrow();
    iSetColor(fColor(18, 39, 60));
    iFilledRectangle(0, 0, bg1.width*2, height);

    checkArrowObstacleCollusion();
    checkRunnerObstacleCollusion();

    /// All Draw Gone
}

void iMouseMove(int mx, int my)
{

}

void iMouse(int button, int state, int mx, int my)
{

}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
	*/
void iKeyboard(unsigned char key)
{
    if(key=='a')
    {

    }
    if(key=='s')
    {
        //runner.state = 0;
    }
    if(key=='d')
    {
        if(runnerState==0)
            runnerState = 1;
    }
    if(key==' ')
    {
        if(runnerState==0)
            runnerState = 2;
    }
}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
	*/
void iSpecialKeyboard(unsigned char key)
{

    if (key == GLUT_KEY_END)
    {
        exit(0);
    }
    if(key == GLUT_KEY_RIGHT)
    {

    }
    if(key == GLUT_KEY_LEFT)
    {

    }
    //place your codes for other keys here
}


void varInitialize()
{

    runner = fObject(IMAGE, width/2-100, initPosY, 115, 112);
    jumpingVel = 4.0*jumpingHeight/(jumpingTStep);
    soil = fObject(REPEATED_IMAGE, 0, 0, 54, 37);
    strcpy(soil.img.location, "soil.bmp");
    bg1 = fObject(REPEATED_IMAGE, 0, 40, 150, 200);
    strcpy(bg1.img.location, "bg.bmp");
    bg2 = fObject(REPEATED_IMAGE, 0, 40, 200, 200);
    strcpy(bg2.img.location, "bg-2.bmp");
    ObsCol1 = fColor(200, 30, 20);

    int i=0;
    for(i=0; i<4; i++)
    {
        obstacle[i] = fObject(RECTANGLE, -1, initPosY, 100, 100);
    }

    /*    *imgUnion       = run;
        *(imgUnion+1)   = jump;
        *(imgUnion+2)   = thr;*/

}



int main()
{
    //place your own initialization codes here.
    iSetTimer(animatingTime, animate);
    varInitialize();

    iInitialize(width, height, "Infinite Runner 2D");
    return 0;
}
