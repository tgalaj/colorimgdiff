#pragma once

#include "BaseComparator.hpp"

class LumaComparator final : public BaseComparator
{
public:
	LumaComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges);
	virtual ~LumaComparator();

	void compare(const std::vector<uint8_t>& ref_img, const std::vector<uint8_t>& src_image) override;
	
	/* Returns MSE value */
	double get_error() const override;

private:
	double m_mse;
};