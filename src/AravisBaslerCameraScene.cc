/*
 * Copyright (c) European XFEL GmbH Schenefeld. All rights reserved.
 */

#include "AravisBaslerBase.hh"

using namespace std;
USING_KARABO_NAMESPACES;


namespace karabo {


    std::string AravisBaslerBase::aravisBaslerScene() {
        const std::string& instanceId = this->getInstanceId();
        std::ostringstream output;
        output << ""
                "<?xml version=\"1.0\"?>"
                "<svg:svg xmlns:krb=\"http://karabo.eu/scene\" xmlns:svg=\"http://www.w3.org/2000/svg\" height=\"654\" width=\"774\" krb:uuid=\"dbad6660-213f-4a1b-b529-f752f46bed34\" krb:version=\"2\">"
                "<svg:rect height=\"41\" width=\"461\" x=\"10\" y=\"10\" krb:class=\"DisplayComponent\" krb:font_size=\"16\" krb:font_weight=\"bold\" krb:keys=\"" << instanceId << ".deviceId\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"21\" width=\"91\" x=\"590\" y=\"10\" krb:background=\"transparent\" krb:class=\"Label\" krb:font=\"Source Sans Pro,10,-1,5,50,0,0,0,0,0\" krb:foreground=\"#000000\" krb:frameWidth=\"0\" krb:text=\"Frame Rate\"/>"
                "<svg:rect height=\"21\" width=\"82\" x=\"679\" y=\"10\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".frameRate.actual\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"21\" width=\"91\" x=\"590\" y=\"30\" krb:background=\"transparent\" krb:class=\"Label\" krb:font=\"Source Sans Pro,10,-1,5,50,0,0,0,0,0\" krb:foreground=\"#000000\" krb:frameWidth=\"0\" krb:text=\"Mean Latency\"/>"
                "<svg:rect height=\"21\" width=\"81\" x=\"680\" y=\"30\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".latency.mean\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"27\" width=\"35\" x=\"10\" y=\"560\" krb:background=\"transparent\" krb:class=\"Label\" krb:font=\"Source Sans Pro,10,-1,5,50,0,0,0,0,0\" krb:foreground=\"#000000\" krb:frameWidth=\"0\" krb:text=\"Gain\"/>"
                "<svg:rect height=\"27\" width=\"91\" x=\"120\" y=\"560\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".gain\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"27\" width=\"106\" x=\"220\" y=\"560\" krb:class=\"EditableApplyLaterComponent\" krb:decimals=\"-1\" krb:keys=\"" << instanceId << ".gain\" krb:widget=\"DoubleLineEdit\"/>"
                "<svg:rect height=\"27\" width=\"90\" x=\"10\" y=\"590\" krb:background=\"transparent\" krb:class=\"Label\" krb:font=\"Source Sans Pro,10,-1,5,50,0,0,0,0,0\" krb:foreground=\"#000000\" krb:frameWidth=\"0\" krb:text=\"Exposure Time\"/>"
                "<svg:rect height=\"27\" width=\"91\" x=\"120\" y=\"590\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".exposureTime\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"27\" width=\"111\" x=\"220\" y=\"590\" krb:class=\"EditableApplyLaterComponent\" krb:decimals=\"-1\" krb:keys=\"" << instanceId << ".exposureTime\" krb:widget=\"DoubleLineEdit\"/>"
                "<svg:rect height=\"25\" width=\"90\" x=\"10\" y=\"620\" krb:background=\"transparent\" krb:class=\"Label\" krb:font=\"Source Sans Pro,10,-1,5,50,0,0,0,0,0\" krb:foreground=\"#000000\" krb:frameWidth=\"0\" krb:text=\"Trigger Source\"/>"
                "<svg:rect height=\"25\" width=\"91\" x=\"120\" y=\"620\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".triggerSource\" krb:widget=\"DisplayLabel\"/>"
                "<svg:rect height=\"25\" width=\"91\" x=\"220\" y=\"620\" krb:class=\"EditableApplyLaterComponent\" krb:keys=\"" << instanceId << ".triggerSource\" krb:widget=\"EditableComboBox\"/>"
                "<svg:rect height=\"41\" width=\"201\" x=\"560\" y=\"560\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".acquire\" krb:requires_confirmation=\"false\" krb:widget=\"DisplayCommand\"/>"
                "<svg:rect height=\"41\" width=\"201\" x=\"560\" y=\"600\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".stop\" krb:requires_confirmation=\"false\" krb:widget=\"DisplayCommand\"/>"
                "<svg:rect height=\"41\" width=\"91\" x=\"460\" y=\"560\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".reset\" krb:requires_confirmation=\"false\" krb:widget=\"DisplayCommand\"/>"
                "<svg:rect height=\"41\" width=\"91\" x=\"460\" y=\"600\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".resetCamera\" krb:requires_confirmation=\"false\" krb:widget=\"DisplayCommand\"/>"
                "<svg:rect height=\"41\" width=\"91\" x=\"490\" y=\"10\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".state\" krb:show_string=\"false\" krb:widget=\"DisplayStateColor\"/>"
                "<svg:rect expression=\"x\" height=\"41\" width=\"91\" x=\"490\" y=\"10\" krb:class=\"DisplayComponent\" krb:keys=\"" << instanceId << ".state\" krb:widget=\"Evaluator\"/>"
                "<svg:rect height=\"461\" width=\"611\" x=\"10\" y=\"60\" krb:aspect_ratio=\"1\" krb:aux_plots=\"0\" krb:class=\"DisplayComponent\" krb:colormap=\"inferno\" krb:keys=\"" << instanceId << ".output.schema.data.image\" krb:roi_tool=\"0\" krb:show_scale=\"1\" krb:widget=\"ImageGraph\" krb:x_label=\"X-axis\" krb:x_scale=\"1.0\" krb:x_translate=\"0.0\" krb:x_units=\"pixels\" krb:y_label=\"Y-axis\" krb:y_scale=\"1.0\" krb:y_translate=\"0.0\" krb:y_units=\"pixels\"/>"
                "</svg:svg>"
                "";

        return output.str();
    }
}
