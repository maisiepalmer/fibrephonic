#ifndef WAVELET2D_H
#define WAVELET2D_H

#include <vector>
#include <complex>
#include <string>



	// === 1D Functions ===
	void* dwt1(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* dyadic_zpad_1d(std::vector<double>&);
	double convol(std::vector<double>&, std::vector<double>&, std::vector<double>&);
	int filtcoef(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void downsamp(std::vector<double>&, int, std::vector<double>&);
	void upsamp(std::vector<double>&, int, std::vector<double>&);
	void circshift(std::vector<double>&, int);
	int sign(int);
	void* idwt1(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	int vecsum(std::vector<double>&, std::vector<double>&, std::vector<double>&);

	// === 1D Symmetric Extension DWT ===
	void* dwt_sym(std::vector<double>&, int, std::string, std::vector<double>&, std::vector<double>&, std::vector<int>&);
	void* dwt1_sym(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* idwt_sym(std::vector<double>&, std::vector<double>&, std::string, std::vector<double>&, std::vector<int>&);
	void* symm_ext(std::vector<double>&, int);
	void* idwt1_sym(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);

	// === 1D Stationary Wavelet Transform ===
	void* swt(std::vector<double>&, int, std::string, std::vector<double>&, int&);
	void* iswt(std::vector<double>&, int, std::string, std::vector<double>&);
	void* per_ext(std::vector<double>&, int);

	// === 2D Basic DWT Functions ===
	void* branch_lp_dn(std::string, std::vector<double>&, std::vector<double>&);
	void* branch_hp_dn(std::string, std::vector<double>&, std::vector<double>&);
	void* branch_lp_hp_up(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* dyadic_zpad_2d(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
	void* dwt_output_dim(std::vector<std::vector<double>>&, int&, int&);
	void* zero_remove(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
	void* getcoeff2d(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<double>&, int&);
	void* idwt2(std::string, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
	void* dwt2(std::string, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
	void* downsamp2(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, int, int);
	void* upsamp2(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, int, int);

	// === 2D Symmetric Extension ===
	void* dwt_2d_sym(std::vector<std::vector<double>>&, int, std::string, std::vector<double>&, std::vector<double>&, std::vector<int>&);
	void* dwt2_sym(std::string, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, std::vector<std::vector<double>>&);
	void* idwt_2d_sym(std::vector<double>&, std::vector<double>&, std::string, std::vector<std::vector<double>>&, std::vector<int>&);
	void* circshift2d(std::vector<std::vector<double>>&, int, int);
	void symm_ext2d(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, int);
	void* dispDWT(std::vector<double>&, std::vector<std::vector<double>>&, std::vector<int>&, std::vector<int>&, int);
	void* dwt_output_dim_sym(std::vector<int>&, std::vector<int>&, int);

	// === 2D Stationary Wavelet Transform ===
	void* swt_2d(std::vector<std::vector<double>>&, int, std::string, std::vector<double>&);
	void* per_ext2d(std::vector<std::vector<double>>&, std::vector<std::vector<double>>&, int);

	// === FFT Functions ===
	double convfft(std::vector<double>&, std::vector<double>&, std::vector<double>&);
	double convfftm(std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* fft(std::vector<std::complex<double>>&, int, unsigned int);
	void* bitreverse(std::vector<std::complex<double>>&);
	void* freq(std::vector<double>&, std::vector<double>&);

	// === Modern Additions (FFTW3 + Symmetric Multilevel) ===
	void* dwt1_sym_m(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* idwt1_sym_m(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* dwt(std::vector<double>&, int, std::string, std::vector<double>&, std::vector<double>&, std::vector<int>&);
	void* idwt(std::vector<double>&, std::vector<double>&, std::string, std::vector<double>&, std::vector<int>&);
	void* dwt_2d(std::vector<std::vector<double>>&, int, std::string, std::vector<double>&, std::vector<double>&, std::vector<int>&);
	void* dwt1_m(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* idwt_2d(std::vector<double>&, std::vector<double>&, std::string, std::vector<std::vector<double>>&, std::vector<int>&);
	void* idwt1_m(std::string, std::vector<double>&, std::vector<double>&, std::vector<double>&);
	void* dwt_output_dim2(std::vector<int>&, std::vector<int>&, int);

 // namespace wavelet2d

#endif // WAVELET2D_H

