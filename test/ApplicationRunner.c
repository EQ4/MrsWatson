#include "ApplicationRunner.h"

static void copyArgumentToArray(void* item, void* userData) {
  ArgumentsCopyData* copyData = (ArgumentsCopyData*)userData;
  char* argString = (char*)item;
  size_t argStringLength = strlen(argString);
  copyData->outArray[copyData->currentIndex] = (char*)malloc(sizeof(char) * argStringLength);
  strncpy(copyData->outArray[copyData->currentIndex], argString, argStringLength);
  copyData->currentIndex++;
}

void runApplicationTest(const char *testName, LinkedList arguments, ReturnCodes expectedResultCode, AnalysisFuncPtr analysisFunction) {
  char* applicationPath;
  char** applicationArguments;
  ArgumentsCopyData argumentsCopyData;
  int resultCode = -1;

  switch(getPlatformType()) {
    case PLATFORM_MACOSX:
      // Xcode should copy the executable to /usr/local/bin during build
      applicationPath = "/usr/local/bin/mrswatson";
      break;
    default:
      logUnsupportedFeature("Application testing");
      return;
  }

#if WINDOWS
#else
  int numArgs = numItemsInList(arguments);
  // Add two extra items to the array, one for the application path and another for a NULL object.
  // These are required for the calls to the execv* functions.
  applicationArguments = (char**)malloc(sizeof(char*) * (numArgs + 2));
  applicationArguments[0] = applicationPath;
  applicationArguments[numArgs + 1] = NULL;
  argumentsCopyData.currentIndex = 1;
  argumentsCopyData.outArray = applicationArguments;
  foreachItemInList(arguments, copyArgumentToArray, &argumentsCopyData);
  printf("-- %s --\n", testName);

  pid_t forkedPid = fork();
  if(forkedPid == 0) {
    resultCode = execvp(applicationPath, applicationArguments);
    exit(resultCode);
  }
  else {
    int statusLoc;
    waitpid(forkedPid, &statusLoc, 0);
    if(WIFEXITED(statusLoc)) {
      resultCode = WEXITSTATUS(statusLoc);
    }
  }
#endif

  if(resultCode == expectedResultCode) {
    testsPassed++;
    printf("-- OK --\n");
  }
  else {
    printf("-- FAIL in test %s. Expected %d, got %d. --\n", testName, expectedResultCode, resultCode);
    testsFailed++;
  }
}