/*
  ==============================================================================

    GestureManager.cpp
    Created: 24 Jun 2025 2:18:38pm
    Author:  Joseph B

  ==============================================================================
*/

#include "GestureManager.h"
#include "BluetoothConnectionManager.h"

GestureManager::GestureManager(shared_ptr<BluetoothConnectionManager> BluetoothConnectionManagerInstance)
    : bluetoothConnection(move(BluetoothConnectionManagerInstance))
{
    DATA.gX = DATA.gY = DATA.gZ = 
    DATA.accX = DATA.accY = DATA.accZ = 
    DATA.jerkX = DATA.jerkY = DATA.jerkZ = 0;

    
    DATA.accXData.resize(DATAWINDOW);
    DATA.accYData.resize(DATAWINDOW);
    DATA.accZData.resize(DATAWINDOW);

    DATA.XData.resize(DATAWINDOW);
    DATA.YData.resize(DATAWINDOW);
    DATA.ZData.resize(DATAWINDOW);

    DATA.xScaled.resize(DATAWINDOW);
    DATA.yScaled.resize(DATAWINDOW);
    DATA.zScaled.resize(DATAWINDOW);

    gesture = NO_GESTURE;
}

GestureManager::~GestureManager()
{
    DATA.accXData.clear();
    DATA.accYData.clear();
    DATA.accZData.clear();

    DATA.XData.clear();
    DATA.YData.clear();
    DATA.ZData.clear();

    DATA.xScaled.clear();
    DATA.yScaled.clear();
    DATA.zScaled.clear();
}

void GestureManager::startPolling() { startTimerHz(IMUINERTIALREFRESH); }
void GestureManager::stopPolling() { stopTimer(); }
void GestureManager::timerCallback() { PollGestures(); } // pollcount++; DBG(pollcount); }

void GestureManager::PollGestures()
{
    getConnectionManagerValues();
    fillDataVectors(&DATA.accXData, &DATA.accYData, &DATA.accZData, &DATA.XData, &DATA.YData, &DATA.ZData, &DATA.gX, &DATA.gY, &DATA.gZ, &DATA.accX, &DATA.accY, &DATA.accZ);

    perform1DWaveletTransform();

    scaleandCopy(DATA.XData, DATA.YData, DATA.ZData, DATA.xScaled, DATA.yScaled, DATA.zScaled);

    /*
    for (size_t i = 0; i < DATAWINDOW; ++i) {
    DBG("Raw: " << DATA.XData[i] << ", Scaled: " << DATA.xScaled[i]);
    }
    */
}

void GestureManager::getConnectionManagerValues()
{
    if (!bluetoothConnection)
    {
        DBG("BluetoothConnectionManager is null!");
        return;
    }

    DATA.gX = bluetoothConnection->getGyroscopeX();
    DATA.gY = bluetoothConnection->getGyroscopeY();
    DATA.gZ = bluetoothConnection->getGyroscopeZ();

    DATA.accX = bluetoothConnection->getAccelerationX();
    DATA.accY = bluetoothConnection->getAccelerationY();
    DATA.accZ = bluetoothConnection->getAccelerationZ();
}

void GestureManager::fillDataVectors(vector<double>* xaccdata,
                                     vector<double>* yaccdata,
                                     vector<double>* zaccdata,
                                     vector<double>* xdata,
                                     vector<double>* ydata,
                                     vector<double>* zdata,
                                                      double* x,
                                                      double* y,
                                                      double* z,
                                                      double* accx,
                                                      double* accy,
                                                      double* accz)
{
    const int scaleVal = 1; // Optional Tweaking 

    // Populate Acc Data
    if (xaccdata->size() >= DATAWINDOW) xaccdata->erase(xaccdata->begin());
    if (yaccdata->size() >= DATAWINDOW) yaccdata->erase(yaccdata->begin());
    if (zaccdata->size() >= DATAWINDOW) zaccdata->erase(zaccdata->begin());

    // Populate Axis Data
    if (xdata->size() >= DATAWINDOW) xdata->erase(xdata->begin());
    if (ydata->size() >= DATAWINDOW) ydata->erase(ydata->begin());
    if (zdata->size() >= DATAWINDOW) zdata->erase(zdata->begin());

    // Push sample (Current Val)
    xaccdata->push_back(static_cast<double>(*accx * scaleVal));
    yaccdata->push_back(static_cast<double>(*accy * scaleVal));
    zaccdata->push_back(static_cast<double>(*accz * scaleVal));

    xdata->push_back(static_cast<double>(*x));
    ydata->push_back(static_cast<double>(*y));
    zdata->push_back(static_cast<double>(*z));
    
}

void GestureManager::decomposeAxis(vector<double>& input,
                                          string wavelet,
                                              int levels,

    vector<double>& coeffs,
    vector<double>& approx,
    vector<double>& detail,
    vector<double>& bookkeeping,
    vector<double>& lengths)
{
    coeffs.clear();
    approx.clear();
    detail.clear();
    bookkeeping.clear();
    lengths.clear();

    dwt(input, levels, wavelet, coeffs, bookkeeping, lengths);

    if (lengths.size() >= 2) {
        approx.assign(coeffs.begin(), coeffs.begin() + lengths[0]);
        detail.assign(coeffs.begin() + lengths[0], coeffs.begin() + lengths[0] + lengths[1]);
    }
}

void GestureManager::reconstructAxis(vector<double>& coeffs,
                                     vector<double>& approx,
                                     vector<double>& detail,
                                     vector<double>& bookkeeping,
                                     vector<double>& lengths,
                                     string wavelet,
                                     vector<double>& reconstructed)
{
    // Resize coeffs to avoid memory overrun
    size_t totalLength = 0;
    for (auto len : lengths) totalLength += static_cast<size_t>(len);
    coeffs.resize(totalLength);

    // Update coeffs with modified approx and detail
    copy(approx.begin(), approx.end(), coeffs.begin());
    copy(detail.begin(), detail.end(), coeffs.begin() + lengths[0]);

    vector<int> idwt_lengths(lengths.begin(), lengths.end());

    idwt(coeffs, bookkeeping, wavelet, reconstructed, idwt_lengths);
}

void GestureManager::softThresholding(std::vector<double>& DetailCoeffs)
{
    // Estimate noise std dev from MAD
    std::vector<double> AbsCoeffs(DetailCoeffs.size());

    for (size_t i = 0; i < DetailCoeffs.size(); ++i)
        AbsCoeffs[i] = std::abs(DetailCoeffs[i]);

    // Calculate true median
    std::sort(AbsCoeffs.begin(), AbsCoeffs.end());
    double Median;
    size_t n = AbsCoeffs.size();
    if (n % 2 == 0)
        Median = (AbsCoeffs[n / 2 - 1] + AbsCoeffs[n / 2]) / 2.0;
    else
        Median = AbsCoeffs[n / 2];

    double NSD = Median / 0.6745; // Noise Standard Deviation
    double Threshold = NSD * std::sqrt(2.0 * std::log(n));

    // Apply soft thresholding
    for (auto& x : DetailCoeffs)
        x = std::copysign(std::max(std::abs(x) - Threshold, 0.0), x);
}


void GestureManager::ModifyWaveletDomain(vector<double>& XApprox, vector<double> XDetail,
                                         vector<double>& YApprox, vector<double> YDetail,
                                         vector<double>& ZApprox, vector<double> ZDetail) 
{
    /* 
    ======================================================================================================================
    Annoyingly.....Separating wavelet modification points caused significant feedback of coeff data.
    This meant data had to be clamped -_-, coeffs now are in a managable range of -1 to 1. This can be changed accordingly 
    and precision adjusted via damping factor.
    ======================================================================================================================
    */

    // Apply Soft thresholding (denoising) to detail coeffs 
    softThresholding(XDetail);
    softThresholding(YDetail);
    softThresholding(ZDetail);

    {
        // Ugly clamping block...

        double damping = 0.95; //control feedback

        auto clampCoeff = [](double& val, double minVal, double maxVal) {
            if (val < minVal) val = minVal;
            else if (val > maxVal) val = maxVal;
            };

        for (auto& val : XApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : XDetail) { val *= damping; clampCoeff(val, -1.0, 1.0); }

        for (auto& val : YApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : YDetail) { val *= damping; clampCoeff(val, -1.0, 1.0); }

        for (auto& val : ZApprox) { val *= damping; clampCoeff(val, -1.0, 1.0); }
        for (auto& val : ZDetail) { val *= damping; clampCoeff(val, -1.0, 1.0); }
    }
    
    // Smooting and transfer functions in modification points here (Applied to axis constantly)

    // Lambda for printing data
    auto vecToString = [](const vector<double>& vec) -> string {
        if (vec.empty())
            return "N/A";
        else
            return std::to_string(vec.back());
        };

    {
        // Modify X....
    }

    std::string line = "X Approx: " + vecToString(DATA.xApprox) +
        ", X Detail: " + vecToString(DATA.xDetail) +
        ", X Reconstructed: " + vecToString(DATA.accXData);
    DBG(line);

    {
        // Modify Y....
    }

    line = "Y Approx: " + vecToString(DATA.yApprox) +
        ", Y Detail: " + vecToString(DATA.yDetail) +
        ", Y Reconstructed: " + vecToString(DATA.accYData);
    //DBG(line);

    {
        // Modify Z....
    }

    line = "Z Approx: " + vecToString(DATA.zApprox) +
        ", Z Detail: " + vecToString(DATA.zDetail) +
        ", Z Reconstructed: " + vecToString(DATA.accZData);
    //DBG(line);

    // What do do immediately if particular gesture is identified (Apply addition gesture specific scaling/ smoothing).
    // MIDI Handler handles primarily what happens upon gesture identification so only tweak slightly.
    switch (gesture) {
    case NO_GESTURE:
        break;
    case PITCH:
        break;
    case ROLL:
        break;
    case YAW:
        break;
    case TAP:
        break;
    case STROKE:
        break;
    default:
        gesture = NO_GESTURE;
        break;
    }
}

GestureManager::Gesture GestureManager::IdentifyGesture(vector<double>& XApprox, vector<double> XDetail,
                                                        vector<double>& YApprox, vector<double> YDetail,
                                                        vector<double>& ZApprox, vector<double> ZDetail)
{
    int minSize = std::min({
    XApprox.size(), XDetail.size(),
    YApprox.size(), YDetail.size(),
    ZApprox.size(), ZDetail.size(),
    static_cast<size_t>(DATAWINDOW / 2)
        });

    // Identify NO_GESTURE 
    for (int i = 0; i < minSize / 2; i++) 
    {
        if (XApprox[i] == 0 && XDetail[i] == 0 &&
            YApprox[i] == 0 && YDetail[i] == 0 &&
            ZApprox[i] == 0 && ZDetail[i] == 0)
        {
            return NO_GESTURE;
        }
    }
}

void GestureManager::perform1DWaveletTransform()
{
    string wavelet = "db4";
    int levels = 1; // Doesn't matter what's here it'll be 1D, has to use other wavelet functions to return other levels

    // Decompose each axis (Transform to Wavelet Domaian)
    decomposeAxis(DATA.accXData, wavelet, levels, DATA.xCoeff, DATA.xApprox, DATA.xDetail, DATA.xBookkeeping, DATA.xLengths);
    decomposeAxis(DATA.accYData, wavelet, levels, DATA.yCoeff, DATA.yApprox, DATA.yDetail, DATA.yBookkeeping, DATA.yLengths);
    decomposeAxis(DATA.accZData, wavelet, levels, DATA.zCoeff, DATA.zApprox, DATA.zDetail, DATA.zBookkeeping, DATA.zLengths);

    // Modify and Observe Trends
    ModifyWaveletDomain(DATA.xApprox, DATA.xDetail, DATA.yApprox, DATA.yDetail, DATA.zApprox, DATA.zDetail);

    // Identify Gestures
    gesture = IdentifyGesture(DATA.xApprox, DATA.xDetail, DATA.yApprox, DATA.yDetail, DATA.zApprox, DATA.zDetail);

    // Reconstruct each axis (Transform back to time domain)
    reconstructAxis(DATA.xCoeff, DATA.xApprox, DATA.xDetail, DATA.xBookkeeping, DATA.xLengths, wavelet, DATA.accXData);
    reconstructAxis(DATA.yCoeff, DATA.yApprox, DATA.yDetail, DATA.yBookkeeping, DATA.yLengths, wavelet, DATA.accYData);
    reconstructAxis(DATA.zCoeff, DATA.zApprox, DATA.zDetail, DATA.zBookkeeping, DATA.zLengths, wavelet, DATA.accZData);  
}

vector<double> GestureManager::normaliseData(double min, double max, vector<double>& input)
{
    vector<double> rescaled;
    rescaled.reserve(input.size());

    if (min == max) {
        // Prevent divide-by-zero, specify midpoint
        rescaled.assign(input.size(), 64.0);  
        return rescaled;
    }
    
    for (double x : input) {
        double normalised = (x - min) / (max - min); // Normalize to [0, 1]
        double scaled = normalised * 126.0 + 1.0;    // Scale to [1, 127]
        rescaled.push_back(scaled);
    }

    return rescaled;
}

void GestureManager::scaleandCopy(vector<double>& xaccdata, vector<double>& yaccdata, vector<double>& zaccdata,
                                  vector<double>& xscale, vector<double>& yscale, vector<double>& zscale)
{
    if (xaccdata.empty() || yaccdata.empty() || zaccdata.empty()) {
        DBG("Data vectors empty!");
        return;
    }

    // Identifies the minimum and maximum of the data in real time. 
    auto [minxT, maxxT] = minmax_element(xaccdata.begin(), xaccdata.end());
    auto [minyT, maxyT] = minmax_element(yaccdata.begin(), yaccdata.end());
    auto [minzT, maxzT] = minmax_element(zaccdata.begin(), zaccdata.end());

    double minxVal = *minxT, maxxVal = *maxxT;
    double minyVal = *minyT, maxyVal = *maxyT;
    double minzVal = *minzT, maxzVal = *maxzT;

    // DBG("MIN: " << minxVal << ", MAX: " << maxxVal);

    xscale = normaliseData(minxVal, maxxVal, xaccdata);
    yscale = normaliseData(minyVal, maxyVal, yaccdata);
    zscale = normaliseData(minzVal, maxzVal, zaccdata);
}




