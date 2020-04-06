/*
 * Author: <parenti>
 *
 * Created on March 17, 2020, 12:20 PM
 *
 * Copyright (c) European XFEL GmbH Hamburg. All rights reserved.
 */

#include "PhotonicScienceCamera.hh"

using namespace std;
USING_KARABO_NAMESPACES

namespace karabo {

    // XXX The following does not compile in Karabo 2.8.1 - too many parameters
    // KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, AravisCamera, PhotonicScienceCamera)
    // XXX Work-around: do not register the base class parameters here,
    //     do it in PhotonicScienceCamera::expectedParameters
    KARABO_REGISTER_FOR_CONFIGURATION(BaseDevice, Device<>, ImageSource, CameraImageSource, PhotonicScienceCamera)

    void PhotonicScienceCamera::expectedParameters(Schema& expected) {
        // Call parent's method, as KARABO_REGISTER_FOR_CONFIGURATION
        // does not compile with too many parameters
        AravisCamera::expectedParameters(expected);

        // Description of not available properties
        const std::string notAvailable("Not available for this camera.");

        SLOT_ELEMENT(expected).key("trigger")
                .displayedName("Software Trigger")
                .description(notAvailable)
                .allowedStates(State::INTERLOCKED) // i.e. never allowed
                .commit();

        // **************************************************************************************************************
        //                                   READ/WRITE HARDWARE PARAMETERS                                             *
        // **************************************************************************************************************

        STRING_ELEMENT(expected).key("pixelSize")
            .alias("PixelSize")
            .tags("genicam")
            .displayedName("Pixel Size")
            .description("This feature indicates the total size in bits of a pixel of the image.")
            .readOnly().initialValue("")
            .commit();


// XXX verify that ROI can be accessed generically...
        OVERWRITE_ELEMENT(expected).key("roi.width")
                .setNewDescription("This value sets the width of the area of interest in pixels. "
                "It must be a multiple of 16. Use '0' for the whole sensor width.")
                .setNewDefaultValue(1920)
                .setNewMinInc(16)
                .commit();
//
// and possibly remove the following.
//        INT32_ELEMENT(expected).key("xOffset")
//                .alias("OffsetX_in_camera")
//                .tags("genicam")
//                .displayedName("X Offset")
//                .description("This value sets the X offset (left offset) for the area of interest in pixels, "
//                "i.e., the distance in pixels between the left side of the sensor and the left side of the image area")
//                .assignmentOptional().defaultValue(0)
//                .unit(Unit::PIXEL)
//                .reconfigurable()
//                .allowedStates(State::UNKNOWN, State::ON)
//                .commit();
//
//        INT32_ELEMENT(expected).key("yOffset")
//                .alias("OffsetY_in_camera")
//                .tags("genicam")
//                .displayedName("Y Offset")
//                .description("This value sets the Y offset (top offset) for the area of interest in pixels, "
//                "i.e., the distance in pixels between the top side of the sensor and the top side of the image area")
//                .assignmentOptional().defaultValue(0)
//                .unit(Unit::PIXEL)
//                .reconfigurable()
//                .allowedStates(State::UNKNOWN, State::ON)
//                .commit();
//
//        INT32_ELEMENT(expected).key("xBinning")
//                .alias("BinningHorizontal")
//                .tags("genicam")
//                .displayedName("X Binning")
//                .description("This feature represents the number of horizontal photo-sensitive cells "
//                "that must be combined (added) together.")
//                .assignmentOptional().defaultValue(1)
//                .unit(Unit::PIXEL)
//                .reconfigurable()
//                .allowedStates(State::UNKNOWN, State::ON)
//                .commit();
//
//        INT32_ELEMENT(expected).key("yBinning")
//                .alias("BinningVertical")
//                .tags("genicam")
//                .displayedName("Y Binning")
//                .description("This feature represents the number of vertical photo-sensitive cells "
//                "that must be combined (added) together.")
//                .assignmentOptional().defaultValue(1)
//                .unit(Unit::PIXEL)
//                .reconfigurable()
//                .allowedStates(State::UNKNOWN, State::ON)
//                .commit();

        STRING_ELEMENT(expected).key("displayMode")
                .alias("Display_mode")
                .tags("genicam")
                .displayedName("Display Mode")
                .description("Select nonstandard display modes. Use to select between low gain, high gain or combined.")
                .assignmentOptional().defaultValue("Combined_low_and_high_gains")
                .options("Low_Gain,High_Gain,Combined_low_and_high_gains")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("exposureMode")
                .alias("ExposureMode")
                .tags("genicam")
                .displayedName("Exposure Mode")
                .description("This feature is used to set the operation mode of the Exposure (or shutter).")
                .assignmentOptional().defaultValue("Timed")
                .options("Timed")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("delayBetweenImages")
                .alias("DELAY_BETWEEN_IMAGES")
                .tags("genicam")
                .displayedName("Delay Between Images")
                .description("Adds additional delay in us between images in free running mode. Use to slow down the frame rate.")
                .assignmentOptional().noDefaultValue()
                .minInc(1).maxInc(100000)
                .unit(Unit::SECOND).metricPrefix(MetricPrefix::MICRO)
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerMode")
                .alias("PSL_TRIGGER_MODE")
                .tags("genicam")
                .displayedName("Trigger Mode")
                .description("Trigger mode selector.")
                .assignmentOptional().defaultValue("Hardware_rising_edge")
                .options("freerun,Hardware_falling_edge,Hardware_rising_edge,SW_Trigger,Pipeline_Marser,Pipeline_slave")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("triggerSource")
                .displayedName("Trigger Source")
                .description(notAvailable)
                .readOnly()
                .commit();

        STRING_ELEMENT(expected).key("triggerActivation")
                .displayedName("Trigger Activation")
                .description(notAvailable)
                .readOnly()
                .commit();

        STRING_ELEMENT(expected).key("powerSaving")
                .alias("Power_saving")
                .tags("genicam")
                .displayedName("Power Saving")
                .description("The power saving mode.")
                .assignmentOptional().defaultValue("Camera_On")
                .options("Camera_On,Cooling_Off,Camera_Off")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

//        INT32_ELEMENT(expected).key("coolerPower")
//                .alias("Cooler_Power")
//                .tags("genicam")
//                .displayedName("Cooler Power")
//                .description("This feature sets cooling power.")
//                .assignmentOptional().noDefaultValue()
//                .minInc(0).maxInc(255)
//                .reconfigurable()
//                .allowedStates(State::UNKNOWN, State::ON)
//                .commit();

        STRING_ELEMENT(expected).key("flickerReduction")
                .alias("flicker_reduction")
                .tags("genicam")
                .displayedName("Flicker Reduction")
                .description("This feature helps to reduce flicker caused by AC lighting. "
                "Only works with exposure times greater than 10ms.")
                .assignmentOptional().defaultValue("OFF")
                .options("OFF,FIFTY_HERTZ,SIXTY_HERTZ")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        INT32_ELEMENT(expected).key("gammaGain")
                .alias("Gamma_gain")
                .tags("genicam")
                .displayedName("Gamma Gain")
                .description("Sets the gain to be applied to the darker parts of the image.")
                .assignmentOptional().defaultValue(1)
                .minInc(1).maxInc(8000)
                .expertAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("bestFit")
                .alias("Best_Fit")
                .tags("genicam")
                .displayedName("Best Fit")
                .description("Stretches the image to use the displays full dynamic range. If gamma = gammy gain then "
                "the dark parts of the image are stretched most. if gamma = one then the image is stretched linearly.")
                .assignmentOptional().defaultValue(false)
                .expertAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("alcAllowAutoBin")
                .alias("ALC_Allow_auto_bin")
                .tags("genicam")
                .displayedName("ALC Allow Auto Bin")
                .description("If true then binning will be used to improve the image at very low light levels. The "
                "image size is not changed by mode. If the Bin filter Feature is Set to True then binning will remain "
                "on all the time regardless of this setting.")
                .assignmentOptional().defaultValue(false)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("columnReduction")
                .alias("Column_reduction")
                .tags("genicam")
                .displayedName("Column reduction")
                .description("To remove remaining column structure from image best applied in darkness. Must be off "
                "to image.")
                .assignmentOptional().defaultValue(false)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("offsetCorrection")
                .alias("Offset_corection")
                .tags("genicam")
                .displayedName("Offset Correction")
                .description("Corrects the image for offset variations.")
                .assignmentOptional().defaultValue("OFFSET_AND_CLAMP_CORRECTED")
                .options("OFF,OFFSET_CORRECTED,OFFSET_AND_CLAMP_CORRECTED")
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("gammaSel")
                .alias("Gamma_sel")
                .tags("genicam")
                .displayedName("Gamma Sel")
                .description("Selects gamma 1 or gamma gain which is controlled by the Gamma_gain_dark control or the best fit.")
                .assignmentOptional().defaultValue("one")
                .options("one,Gamma_gain,Gamma_gain_low")
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("brightPixelCorrection")
                .alias("Bright_Pixel_Corection")
                .tags("genicam")
                .displayedName("Bright Pixel Correction")
                .description("Turns on Bright pixel correction.")
                .assignmentOptional().defaultValue(true)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("binFilter")
                .alias("Bin_Filter")
                .tags("genicam")
                .displayedName("Bin Filter")
                .description("If true image is binned 2x2 then rescaled to original size.")
                .assignmentOptional().defaultValue(false)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("bpRemovalStrong")
                .alias("BP_REMOVEL_STRONG")
                .tags("genicam")
                .displayedName("BP Removal Strong")
                .description("Will correct more low level bright pixels when set But also erode edges.")
                .assignmentOptional().defaultValue(true)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        BOOL_ELEMENT(expected).key("preampGainMode")
                .alias("Preamp_Gain_Mode")
                .tags("genicam")
                .displayedName("Preamp Gain Mode")
                .assignmentOptional().defaultValue(false)
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        STRING_ELEMENT(expected).key("pixelSpeed")
                .alias("Pixel_Speed")
                .tags("genicam")
                .displayedName("Pixel Speed")
                .description("Sets the pixel speed in MHz. Only preset values are supported.")
                .assignmentOptional().defaultValue("MHz50")
                .options("MHz50,MHz100")
                .adminAccess()
                .reconfigurable()
                .allowedStates(State::UNKNOWN, State::ON)
                .commit();

        // **************************************************************************************************************
        //                                   READ ONLY HARDWARE PARAMETERS                                              *
        // **************************************************************************************************************

        STRING_ELEMENT(expected).key("cameraVersion")
                .alias("DeviceVersion")
                .tags("genicam")
                .displayedName("Camera Version")
                .description("This feature provides the version of the device.")
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("sensorWidth")
                .alias("SensorWidth")
                .tags("genicam")
                .displayedName("Sensor Width")
                .description("This feature indicates the effective width of the sensor in pixels.")
                .unit(Unit::PIXEL)
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("sensorHeight")
                .alias("SensorHeight")
                .tags("genicam")
                .displayedName("Sensor Height")
                .description("This feature indicates the effective height of the sensor in pixels.")
                .unit(Unit::PIXEL)
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("maxWidth")
                .alias("WidthMax")
                .tags("genicam")
                .displayedName("Max Width")
                .description("This feature represents the maximum width (in pixels) of the image after "
                "horizontal binning, decimation or any other function changing the horizontal dimensions "
                "of the image.")
                .unit(Unit::PIXEL)
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("maxHeight")
                .alias("HeightMax")
                .tags("genicam")
                .displayedName("Max Height")
                .description("This feature represents the maximum height (in pixels) of the image after "
                "vertical binning, decimation or any other function changing the vertical dimensions "
                "of the image.")
                .unit(Unit::PIXEL)
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("ccdTemperature")
                .alias("CCD_TEMPERATURE")
                .tags("poll")
                .displayedName("CCD Temperature")
                .description("This feature represents the CCD temperature.")
                .readOnly()
                .commit();

        INT32_ELEMENT(expected).key("pldTemperature")
                .alias("PLD_TEMPERATURE")
                .tags("poll")
                .displayedName("PCB Temperature")
                .description("This feature represents the PCB temperature.")
                .readOnly()
                .commit();

    }

    PhotonicScienceCamera::PhotonicScienceCamera(const karabo::util::Hash& config) : AravisCamera(config) {
        m_arv_camera_trigger = false; // Trigger properties to be accessed from non-standard paths
    }

    PhotonicScienceCamera::~PhotonicScienceCamera() {
    }

    void PhotonicScienceCamera::configure(karabo::util::Hash& configuration) {
        if (configuration.has("roi.width")) {
            // width must be multiple of 16
            int width = configuration.get<int>("roi.width");
            if (width % 16 != 0) {
                width = 16 * std::round(width / 16.);
                configuration.set("roi.width", width);
            }
        }

        // Call parent's function
        AravisCamera::configure(configuration);
    }

    void PhotonicScienceCamera::trigger() {
        // The camera possibly does not support software trigger
        // XXX verify
    }

} // namespace karabo
