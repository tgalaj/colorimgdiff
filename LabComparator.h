#pragma once

#include "BaseComparator.h"

class LabComparator final : public BaseComparator
{
public:
	LabComparator(const tinycolormap::ColormapType& colormap_type, const std::string& out_filename, unsigned width, unsigned height, int interpolation_ranges);
	virtual ~LabComparator();

	void compare(const std::vector<uint8_t>& ref_img, const std::vector<uint8_t>& src_image) override;

	/* Returns Delta E value */
	double get_error() const override;

private:
	double m_delta_e;
};