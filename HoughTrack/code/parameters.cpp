/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 ******************************************************************************/

#include "parameters.h"

Parameters::Parameters(const std::string& confFile) : m_filename(confFile)
{
	try{
		m_configFile.readFile(m_filename.c_str());
   } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

Parameters::~Parameters()
{
}

int Parameters::readIntParameter(std::string param_name) const
{
    try{
        return (int)m_configFile.lookup(param_name);
    } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

double Parameters::readDoubleParameter(std::string param_name) const
{
    try{
        return (double)m_configFile.lookup(param_name);
    } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

std::string Parameters::readStringParameter(std::string param_name) const
{
    try{
        return std::string((const char*)m_configFile.lookup(param_name));
    } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

cv::Size Parameters::readSizeParameter(std::string param_name) const
{
    try{

	assert( m_configFile.lookup(param_name).getLength() >= 2 );

        int* ret = new int[2];
		for(int i = 0; i < 2; i++)
			ret[i] = m_configFile.lookup(param_name)[i];

        cv::Size size(ret[0], ret[1]);
        delete[] ret;
        return size;

    } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

cv::Rect Parameters::readRectParameter(std::string param_name) const
{
    try{

	assert( m_configFile.lookup(param_name).getLength() >= 4 );

    	int* ret = new int[4];
		for(int i = 0; i < 4; i++)
			ret[i] = m_configFile.lookup(param_name)[i];

        cv::Rect rect(ret[0], ret[1], ret[2], ret[3]);
        delete[] ret;
        return rect;

    } catch (libconfig::ParseException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "ParseException at Line " << e.getLine() << ": " << e.getError() << std::endl;
        exit(EXIT_FAILURE);
    } catch (libconfig::SettingException& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "SettingException at " << e.getPath() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "Error reading Configuration file (" << m_filename << ")!" << std::endl;
        std::cout << "Unknown Exception" << std::endl;
        exit(EXIT_FAILURE);
	}
}

bool Parameters::settingExists(std::string param_name) const
{
	return m_configFile.exists(param_name);
}

