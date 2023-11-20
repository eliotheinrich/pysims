#pragma once

#include <DataFrame.hpp>
#include <pffft.hpp>
#include <complex>

using fft_plan = pffft::Fft<double>;

class InterfaceSampler {
	private:	
		int system_size;

		bool sample_surface;
		bool sample_surface_avg;

		int num_bins;
		int min_av;
		int max_av;
		bool sample_avalanche_sizes;
		std::vector<int> avalanche_sizes;

		bool sample_structure_function;
		bool transform_fluctuations;

		int max_width;
		bool sample_rugosity;
		bool sample_roughness;

        int get_bin_idx(double s) const {
            if ((s < min_av) || (s > max_av)) {
                std::string error_message = std::to_string(s) + " is not between " + std::to_string(min_av) + " and " + std::to_string(max_av) + ". \n";
                throw std::invalid_argument(error_message);
            }

            double bin_width = static_cast<double>(max_av - min_av)/num_bins;
			int idx = static_cast<int>((s - min_av) / bin_width);
            return idx;
        }

	public:
		InterfaceSampler()=default;

		InterfaceSampler(Params& params) {
			system_size = get<int>(params, "system_size");
			
			sample_surface = get<int>(params, "sample_surface", true);
			sample_surface_avg = get<int>(params, "sample_surface_avg", false);

			max_width = get<int>(params, "max_width", system_size/2);
			sample_rugosity = get<int>(params, "sample_rugosity", false);
			sample_roughness = get<int>(params, "sample_roughness", false);

			sample_structure_function = get<int>(params, "sample_structure_function", false);
			transform_fluctuations = get<int>(params, "transform_fluctuations", false);

			num_bins = get<int>(params, "num_bins", 100);
			min_av = get<int>(params, "min_av", 1);
			max_av = get<int>(params, "max_av", 100);

			if (max_av <= min_av)
				throw std::invalid_argument("max_av must be greater than min_av");

			sample_avalanche_sizes = get<int>(params, "sample_avalanche_sizes", false);

			avalanche_sizes = std::vector<int>(num_bins);
		}

		std::vector<double> structure_function(const std::vector<int>& surface) const {
			int N = surface.size();
			fft_plan fft(N);

			if (!fft.isValid()) {
				std::string error_message = "surface.size() = " + std::to_string(N) + " is not valid for fft.";
				throw std::invalid_argument(error_message);
			}

			auto input = fft.valueVector();
			auto output = fft.spectrumVector();

			double Ns = std::sqrt(N);

			if (transform_fluctuations) {
				double hb = surface_avg(surface);
				for (int i = 0; i < N; i++)
					input[i] = static_cast<double>(surface[i] - hb)/Ns;
			} else {
				for (int i = 0; i < N; i++)
					input[i] = static_cast<double>(surface[i])/Ns;
			}

			fft.forward(input, output);


			int spectrum_size = fft.getSpectrumSize();

			std::vector<double> sk(spectrum_size+1);
			for (int i = 1; i < spectrum_size; i++)
				sk[i] = std::real(output[i]*std::conj(output[i]));

			sk[0] = std::pow(output[0].real(), 2);
			sk[spectrum_size] = std::pow(output[0].imag(), 2);

			return sk;
		}

		void record_size(int s) {
			if (s >= min_av && s <= max_av) {
				int idx = get_bin_idx(s);
				avalanche_sizes[idx]++;
			}
		}

		double roughness(const std::vector<int> &surface) const {
			int num_sites = surface.size();

			return roughness_window(num_sites/2, surface);
		}

		double roughness_window(int width, const std::vector<int>& surface) const {
			double w = 0.0;
			double hb = surface_avg_window(width, surface);

			int num_sites = surface.size();
			for (int i = num_sites/2 - width; i < num_sites/2 + width; i++)
				w += std::pow(surface[i] - hb, 2);

			return w/(2.0*width);
		}

		double surface_avg(const std::vector<int>& surface) const {
			int num_sites = surface.size();

			return surface_avg_window(num_sites/2, surface);
		}

		double surface_avg_window(int width, const std::vector<int> &surface) const {
			int num_sites = surface.size();
			if (2*width > num_sites) {
				std::string error_message = "width = " + std::to_string(width) + 
											" is too large for the size of the surface = " + std::to_string(num_sites) + ".";
				throw std::invalid_argument(error_message);
			}

			int sum = 0;
			for (int i = num_sites/2 - width; i < num_sites/2 + width; i++)
				sum += surface[i];

			return static_cast<double>(sum)/(2.0*width);
		}

		void add_surface_samples(data_t &samples, const std::vector<int>& surface) const {
			int num_sites = surface.size();
			for (int i = 0; i < num_sites; i++)
				samples.emplace("surface_" + std::to_string(i), surface[i]);
		}

		void add_avalanche_samples(data_t &samples) {
			int total_avalanches = 0;
			for (int i = 0; i < num_bins; i++)
				total_avalanches += avalanche_sizes[i];

			if (total_avalanches == 0) {
				for (int i = 0; i < num_bins; i++) 
					samples.emplace("avalanche_" + std::to_string(i), 0.0);
			} else {
				for (int i = 0; i < num_bins; i++) 
					samples.emplace("avalanche_" + std::to_string(i), static_cast<double>(avalanche_sizes[i])/total_avalanches);
			}

			avalanche_sizes = std::vector<int>(num_bins, 0);
		}

		void add_structure_function_samples(data_t &samples, const std::vector<int> &surface) const {
			std::vector<double> sk = structure_function(surface);
			for (int j = 0; j < sk.size(); j++)
				samples.emplace("structure_" + std::to_string(j), sk[j]);
		}

		void add_rugosity_samples(data_t& samples, const std::vector<int>& surface) const {
			int num_sites = surface.size();
			for (int width = 1; width < std::min(num_sites/2, max_width); width++)
				samples.emplace("roughness_" + std::to_string(width), std::pow(roughness_window(width, surface), 2));
		}

		void add_samples(data_t &samples, const std::vector<int>& surface) {
			if (sample_surface)
				add_surface_samples(samples, surface);
			
			if (sample_surface_avg)
				samples.emplace("surface_avg", surface_avg(surface));

			if (sample_rugosity)
				add_rugosity_samples(samples, surface);

			if (sample_roughness)
				samples.emplace("roughness", roughness(surface));

			if (sample_avalanche_sizes)
				add_avalanche_samples(samples);

			if (sample_structure_function)
				add_structure_function_samples(samples, surface);
		}
};