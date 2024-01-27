#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>     
#include <unistd.h>    
#include <sys/ioctl.h> 

#define mDeviceName                 "/dev/virtual_temp_sensor" 
#define mCommandFahrenheit          _IOR('k', 1, int)

int main() 
{
    int fileDesc;  // File descriptor
    int CelsiusTempValue;
    int FahrenheitTempValue;

    // Open the mDeviceName
    fileDesc = open(mDeviceName, O_RDONLY);
    if (fileDesc < 0) {
        perror("Failed to open the mDeviceName...");
        return -1;
    }

    // Read from the mDeviceName
    if (read(fileDesc, &CelsiusTempValue, sizeof(CelsiusTempValue)) < 0) {
        perror("Failed to read the data...");
        return -1;
    }

    // Send a specific request
    ioctl(fileDesc, mCommandFahrenheit, &FahrenheitTempValue);

    printf("Current Temperature : %d Celsius, %d Fahrenheit\n", CelsiusTempValue, FahrenheitTempValue);

    // Close the mDeviceName
    close(fileDesc);

    return 0;
}
