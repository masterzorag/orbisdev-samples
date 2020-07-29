

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ps4sdk.h>
#include <orbislink.h>
#include <orbisGl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <libkernel.h>  //sceKernelIccSetBuzzer


OrbisPadConfig *confPad;
bool flag=true;

    

void updateController()
{
    unsigned int buttons=0;
    int ret = orbisPadUpdate();
    if(ret==0)
    {
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1|ORBISPAD_R2) || orbisPadGetButtonHold(ORBISPAD_L1|ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L1R2 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L1|ORBISPAD_R2);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L2|ORBISPAD_R1) || orbisPadGetButtonHold(ORBISPAD_L2|ORBISPAD_R1) )
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Combo L2R1 pressed\n");
            buttons=orbisPadGetCurrentButtonsPressed();
            buttons&= ~(ORBISPAD_L2|ORBISPAD_R1);
            orbisPadSetCurrentButtonsPressed(buttons);
        }
        if(orbisPadGetButtonPressed(ORBISPAD_UP))// || orbisPadGetButtonHold(ORBISPAD_UP))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Up pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_DOWN))// || orbisPadGetButtonHold(ORBISPAD_DOWN))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Down pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_RIGHT))// || orbisPadGetButtonHold(ORBISPAD_RIGHT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Right pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_LEFT))// || orbisPadGetButtonHold(ORBISPAD_LEFT))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Left pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_TRIANGLE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Triangle pressed exit\n");

            flag=0;  // exit app
        }
        if(orbisPadGetButtonPressed(ORBISPAD_CIRCLE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Circle pressed\n");         
        }
        if(orbisPadGetButtonPressed(ORBISPAD_CROSS))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Cross pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_SQUARE))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"Square pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"L1 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_L2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"L2 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R1))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"R1 pressed\n");
        }
        if(orbisPadGetButtonPressed(ORBISPAD_R2))
        {
            debugNetPrintf(DEBUGNET_DEBUG,"R2 pressed\n");
        }
    }
}

void finishApp()
{
    //orbisAudioFinish();
    //orbisKeyboardFinish();
    //orbisGlFinish();
    orbisPadFinish();
    orbisNfsFinish();
}

static bool initAppGl()
{
    int ret = orbisGlInit(ATTR_ORBISGL_WIDTH,ATTR_ORBISGL_HEIGHT);
    if(ret>0)
    {
        glViewport(0, 0, ATTR_ORBISGL_WIDTH, ATTR_ORBISGL_HEIGHT);
        ret=glGetError();
        if(ret)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[%s] glViewport failed: 0x%08X\n",__FUNCTION__,ret);
            return false;
        }
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f); //blue RGBA
        ret=glGetError();
        if(ret)
        {
            debugNetPrintf(DEBUGNET_ERROR,"[%s] glClearColor failed: 0x%08X\n",__FUNCTION__,ret);
            return false;
        }
        return true;
    }
    return false;
}

bool initApp()
{
    //orbisNfsInit(NFSEXPORT);
    //orbisFileInit();
//  int ret=initOrbisLinkApp();

    sceSystemServiceHideSplashScreen();


    confPad=orbisPadGetConf(); 

    if( ! initAppGl() ) return false;

    return true;
}


/// for timing, fps
#define WEN  (2048)
unsigned int frame   = 1,
             time_ms = 0;

int main(int argc, char *argv[])
{
    int ret = sceKernelIccSetBuzzer(1);

    sleep(3);
    /*
        prepare pad and piglet modules in advance,
        read /data/orbislink/orbislink_config.ini for debugnet

        (no nfs, audio init yet...)
    */

    // debugNetInit("10.0.0.2", 18198, 3);
    ret = initOrbisLinkAppVanillaGl(); // will call loadModulesVanillaGl();
    if(ret)
    {
        debugNetPrintf(3,"%s: initOrbisLinkAppVanillaGl() ret: %d\n",__FUNCTION__, ret);
        sceKernelIccSetBuzzer(2); // notify error
    }

    // init libraries
    flag=initApp();


    /// reset timer
    time_ms = get_time_ms();


    /// enter main render loop
    while(flag)
    {
        updateController();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ret = glGetError();
        if(ret) {
            debugNetPrintf(DEBUGNET_ERROR,"[ORBIS_GL] glClear failed: 0x%08X\n", ret);
            //goto err;
        }



        /// get timing, fps
        if( ! (frame %WEN) )
        {
            unsigned int now = get_time_ms();
            debugNetPrintf(DEBUGNET_INFO,"frame: %d, took: %ums, fps: %.3f\n", frame, now - time_ms,
                                                     ((double)WEN / (double)(now - time_ms) * 1000.f));
            time_ms = now;
        }
        frame++;

        orbisGlSwapBuffers();  /// flip frame


        sceKernelUsleep(1000);
    }
    
    // destructors
    finishOrbisLinkApp();

    return 0;

    //exit(EXIT_SUCCESS);
}
