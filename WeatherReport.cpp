#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include "SMTPClient.h"

int getMax(std::vector<std::string> numbers)
{
    int max = std::stoi(numbers[1]);
    for (int i = 2; i < numbers.size(); ++i)
    {
        if (std::stoi(numbers[i]) > max)
        {
            max = std::stoi(numbers[i]);
        }
    }

    return max;
}

int getMin(std::vector<std::string> numbers)
{
    int min = std::stoi(numbers[1]);
    for (int i = 2; i < numbers.size(); ++i)
    {
        if (std::stoi(numbers[i]) < min)
        {
            min = std::stoi(numbers[i]);
        }
    }

    return min;
}

int getMedian(std::vector<std::string> numbers)
{
    numbers.erase(numbers.begin());
    sort(numbers.begin(), numbers.end());

    int median = 0;
    if ((numbers.size() % 2) == 0)
    {
        median = (std::stoi(numbers[(numbers.size() / 2) - 1]) + std::stoi(numbers[numbers.size() / 2])) / 2;

    }
    else
    {
        median = std::stoi(numbers[numbers.size() / 2]);
    }

    return median;
}

void getTheMostAppropriateDay(const std::string& fileName, std::vector<int> days)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cout << "Failed to open the file.\n";
        return;
    }

    std::ofstream outputFile("WeatherReport.csv");
    if (!outputFile.is_open())
    {
        std::cout << "Failed to open the file for writing.\n";
        return;
    }

    std::string line;
    int lineCount = 0;
    int theMostAppropriateDay = days[0];
    while (std::getline(file, line))
    {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ','))
        {
            row.push_back(token);
        }

        if (days.size() != 1)
        {
            for (int i = 1; i < days.size(); ++i)
            {
                if (row[0] == "Temperature (C)" || row[0] == "Wind (m/s)" || row[0] == "Humidity (%)" || row[0] == "Precipitation (%)")
                {
                    int currentDayValue = std::stoi(row[days[i]]);
                    int mostAppropriateDayValue = std::stoi(row[theMostAppropriateDay]);
                    if (currentDayValue < mostAppropriateDayValue)
                    {
                        theMostAppropriateDay = days[i];
                    }
                }
            }
        }

        if (row[0] == "Day/Parameter")
        {
            outputFile << row[0] << ",Average value, Max value, Min value, Median value," << row[theMostAppropriateDay] << '\n';
            ++lineCount;
            continue;
        }

        int sum = 0;
        int average = 0;
        int max = 0;
        int min = 0;
        int median = 0;

        for (int i = 1; i < row.size(); ++i)
        {
            if (row[0] == "Temperature (C)" || row[0] == "Wind (m/s)" || row[0] == "Humidity (%)" || row[0] == "Precipitation (%)")
            {
                sum += std::stoi(row[i]);
            }
        }

        if (row[0] == "Temperature (C)" || row[0] == "Wind (m/s)" || row[0] == "Humidity (%)" || row[0] == "Precipitation (%)")
        {
            average = sum / (row.size() - 1); 
            max = getMax(row); 
            min = getMin(row); 
            median = getMedian(row); 
            outputFile << row[0] << "," << average << "," << max << "," << min << "," << median << "," << row[theMostAppropriateDay] << '\n';
        }
        else
        {
            outputFile << row[0] << ",\t,\t,\t,\t," << row[theMostAppropriateDay] << '\n';
        }

        ++lineCount;
    }

    file.close();
    outputFile.close();
}

void getWeatherReport(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cout << "Failed to open the file.\n";
        return;
    }

    std::ofstream outputFile("WeatherReport.csv");
    if (!outputFile.is_open())
    {
        std::cout << "Failed to open the file for writing.\n";
        return;
    }

    std::string line;
    std::vector<std::vector<std::string>> data;
    std::vector<int> days;

    while (std::getline(file, line))
    {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string token;

        while (std::getline(ss, token, ','))
        {
            row.push_back(token);
        }

        for (int i = 1; i < row.size(); ++i)
        {
            if (row[0] == "Temperature (C)" && (std::stoi(row[i]) > 2 && std::stoi(row[i]) < 31))
            {
                days.push_back(i);
            }

            if (row[0] == "Wind (m/s)" && (std::stoi(row[i]) > 10))
            {
                auto index = std::find(days.begin(), days.end(), i);
                if (index != days.end())
                {
                    days.erase(index);
                }
            }

            if (row[0] == "Humidity (%)" && (std::stoi(row[i]) >= 60))
            {
                auto index = std::find(days.begin(), days.end(), i);
                if (index != days.end())
                {
                    days.erase(index);
                }
            }

            if (row[0] == "Precipitation (%)" && (std::stoi(row[i]) != 0))
            {
                auto index = std::find(days.begin(), days.end(), i);
                if (index != days.end())
                {
                    days.erase(index);
                }
            }

            if (row[0] == "Lightning" && row[i] != "No")
            {
                auto index = std::find(days.begin(), days.end(), i);
                if (index != days.end())
                {
                    days.erase(index);
                }
            }

            if (row[0] == "Clouds" && (row[i] == "Cumulus" || row[i] == "Nimbus"))
            {
                auto index = std::find(days.begin(), days.end(), i);
                if (index != days.end())
                {
                    days.erase(index);
                }
            }
        }

        data.push_back(row);
    }

    if (days.empty())
    {
        std::cout << "No appropriate days for launching.\n";
        return;
    }

    getTheMostAppropriateDay(fileName, days);

    file.close();
    outputFile.close();
}

bool isValidEmail(const std::string& email)
{
    const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");

    return std::regex_match(email, pattern);
}

int main()
{
    std::string fileName;
    std::cout << "Enter a path to a file:\n";
    std::cin >> fileName; //C:\Users\Irinka\Desktop\WeatherConditions.csv

    if (fileName.length() < 4 || fileName.substr(fileName.length() - 4) != ".csv")
    {
        std::cout << "The file must be a .csv file.\n";
    }

    getWeatherReport(fileName);

    std::string senderEmail;
    std::cout << "Enter a sender email:\n";
    std::cin >> senderEmail;

    if (!isValidEmail(senderEmail))
    {
        std::cout << "This email is invalid.\n";
    }

    std::string password;
    std::cout << "Enter a password:\n";
    std::cin >> password;

    std::string receiverEmail;
    std::cout << "Enter a receiver email:\n";
    std::cin >> receiverEmail;

    if (!isValidEmail(receiverEmail))
    {
        std::cout << "This email is invalid.\n";
    }

    SMTPClient client("smtp.abv.bg", password);
    client.sendEmail(senderEmail, receiverEmail, "C:\\Users\\Irinka\\Desktop\\WeatherReport.csv");
}
