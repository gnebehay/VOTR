/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include <string>
#include <vector>
#include <iostream>
#include <exception>
#include <cstdlib>
#include <libconfig.h++>

#include "cv.h"

class Parameters
{
public:
	Parameters(const std::string& confFile);
	virtual ~Parameters();

	int readIntParameter(std::string param_name) const;
	double readDoubleParameter(std::string param_name) const;
	std::string readStringParameter(std::string param_name) const;

	cv::Size readSizeParameter(std::string param_name) const;
	cv::Rect readRectParameter(std::string param_name) const;

	bool settingExists(std::string param_name) const;

protected:
	libconfig::Config m_configFile;
	std::string m_filename;

};

#endif // PARAMETERS_H_
