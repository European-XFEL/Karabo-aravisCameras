#ifndef KARABO_ARAVISIDSCAMERA_HH
#define KARABO_ARAVISIDSCAMERA_HH

#include <string>
#include "AravisCamera.hh"
#include "version.hh"

namespace karabo {

class AravisIdsCamera final : public AravisCamera {
public:
    KARABO_CLASSINFO(AravisIdsCamera, "AravisIdsCamera", ARAVISCAMERAS_PACKAGE_VERSION)

    static void expectedParameters(karabo::data::Schema& expected);
    explicit AravisIdsCamera(const karabo::data::Hash& config);
    ~AravisIdsCamera() override = default;

protected:
    bool is_flip_x_available() const override;
    bool is_flip_y_available() const override;
    std::string get_frame_rate_enable_parameter_name() const override;

private:
    void postAcquisitionStop() override;

    void resetCamera() override;
};

} // namespace karabo

#endif // KARABO_ARAVISIDSCAMERA_HH
