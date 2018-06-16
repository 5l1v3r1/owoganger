#pragma once
#include <Windows.h>
#include <KtmW32.h>

#define MAX_BUF_SIZE 0x1000
#define PE_HEADER 0x1000
#define GDI_HANDLE_BUFFER_SIZE      34
#define RTL_USER_PROC_PARAMS_NORMALIZED 0x00000001

	typedef struct _LSA_UNICODE_STRING {
		USHORT Length;
		USHORT MaximumLength;
		PWSTR  Buffer;
	} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING, UNICODE_STRING, *PUNICODE_STRING;

	typedef struct _PEB_FREE_BLOCK
	{
		struct _PEB_FREE_BLOCK *Next;
		ULONG Size;

	} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

	typedef struct _STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PCHAR  Buffer;

	} STRING, *PSTRING;
	typedef struct _RTL_DRIVE_LETTER_CURDIR
	{
		USHORT Flags;
		USHORT Length;
		ULONG  TimeStamp;
		STRING DosPath;

	} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

	typedef struct _CURDIR
	{
		UNICODE_STRING DosPath;
		HANDLE Handle;

	} CURDIR, *PCURDIR;

	typedef struct _RTL_USER_PROCESS_PARAMETERS
	{
		ULONG MaximumLength;                            // Should be set before call RtlCreateProcessParameters
		ULONG Length;                                   // Length of valid structure
		ULONG Flags;                                    // Currently only PPF_NORMALIZED (1) is known:
														//  - Means that structure is normalized by call RtlNormalizeProcessParameters
		ULONG DebugFlags;

		PVOID ConsoleHandle;                            // HWND to console window associated with process (if any).
		ULONG ConsoleFlags;
		HANDLE StandardInput;
		HANDLE StandardOutput;
		HANDLE StandardError;

		CURDIR CurrentDirectory;                        // Specified in DOS-like symbolic link path, ex: "C:/WinNT/SYSTEM32"
		UNICODE_STRING DllPath;                         // DOS-like paths separated by ';' where system should search for DLL files.
		UNICODE_STRING ImagePathName;                   // Full path in DOS-like format to process'es file image.
		UNICODE_STRING CommandLine;                     // Command line
		PVOID Environment;                              // Pointer to environment block (see RtlCreateEnvironment)
		ULONG StartingX;
		ULONG StartingY;
		ULONG CountX;
		ULONG CountY;
		ULONG CountCharsX;
		ULONG CountCharsY;
		ULONG FillAttribute;                            // Fill attribute for console window
		ULONG WindowFlags;
		ULONG ShowWindowFlags;
		UNICODE_STRING WindowTitle;
		UNICODE_STRING DesktopInfo;                     // Name of WindowStation and Desktop objects, where process is assigned
		UNICODE_STRING ShellInfo;
		UNICODE_STRING RuntimeData;
		RTL_DRIVE_LETTER_CURDIR CurrentDirectores[0x20];
		ULONG EnvironmentSize;
	} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

	typedef struct _PEB_LDR_DATA
	{
		ULONG Length;
		BOOLEAN Initialized;
		HANDLE SsHandle;
		LIST_ENTRY InLoadOrderModuleList;               // Points to the loaded modules (main EXE usually)
		LIST_ENTRY InMemoryOrderModuleList;             // Points to all modules (EXE and all DLLs)
		LIST_ENTRY InInitializationOrderModuleList;
		PVOID      EntryInProgress;

	} PEB_LDR_DATA, *PPEB_LDR_DATA;

	
	typedef struct _OBJECT_ATTRIBUTES {
		ULONG           Length;
		HANDLE          RootDirectory;
		PUNICODE_STRING ObjectName;
		ULONG           Attributes;
		PVOID           SecurityDescriptor;
		PVOID           SecurityQualityOfService;
	} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;
	typedef struct _PEB
	{
		BOOLEAN InheritedAddressSpace;      // These four fields cannot change unless the
		BOOLEAN ReadImageFileExecOptions;   //
		BOOLEAN BeingDebugged;              //
		BOOLEAN SpareBool;                  //
		HANDLE Mutant;                      // INITIAL_PEB structure is also updated.

		PVOID ImageBaseAddress;
		PPEB_LDR_DATA Ldr;
		PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
		PVOID SubSystemData;
		PVOID ProcessHeap;
		PVOID FastPebLock;
		PVOID FastPebLockRoutine;
		PVOID FastPebUnlockRoutine;
		ULONG EnvironmentUpdateCount;
		PVOID KernelCallbackTable;
		HANDLE SystemReserved;
		PVOID  AtlThunkSListPtr32;
		PPEB_FREE_BLOCK FreeList;
		ULONG TlsExpansionCounter;
		PVOID TlsBitmap;
		ULONG TlsBitmapBits[2];         // relates to TLS_MINIMUM_AVAILABLE
		PVOID ReadOnlySharedMemoryBase;
		PVOID ReadOnlySharedMemoryHeap;
		PVOID *ReadOnlyStaticServerData;
		PVOID AnsiCodePageData;
		PVOID OemCodePageData;
		PVOID UnicodeCaseTableData;

		//
		// Useful information for LdrpInitialize

		ULONG NumberOfProcessors;
		ULONG NtGlobalFlag;

		//
		// Passed up from MmCreatePeb from Session Manager registry key
		//

		LARGE_INTEGER CriticalSectionTimeout;
		ULONG HeapSegmentReserve;
		ULONG HeapSegmentCommit;
		ULONG HeapDeCommitTotalFreeThreshold;
		ULONG HeapDeCommitFreeBlockThreshold;

		//
		// Where heap manager keeps track of all heaps created for a process
		// Fields initialized by MmCreatePeb.  ProcessHeaps is initialized
		// to point to the first free byte after the PEB and MaximumNumberOfHeaps
		// is computed from the page size used to hold the PEB, less the fixed
		// size of this data structure.
		//

		ULONG NumberOfHeaps;
		ULONG MaximumNumberOfHeaps;
		PVOID *ProcessHeaps;

		//
		//
		PVOID GdiSharedHandleTable;
		PVOID ProcessStarterHelper;
		PVOID GdiDCAttributeList;
		PVOID LoaderLock;

		//
		// Following fields filled in by MmCreatePeb from system values and/or
		// image header. These fields have changed since Windows NT 4.0,
		// so use with caution
		//

		ULONG OSMajorVersion;
		ULONG OSMinorVersion;
		USHORT OSBuildNumber;
		USHORT OSCSDVersion;
		ULONG OSPlatformId;
		ULONG ImageSubsystem;
		ULONG ImageSubsystemMajorVersion;
		ULONG ImageSubsystemMinorVersion;
		ULONG ImageProcessAffinityMask;
		ULONG GdiHandleBuffer[GDI_HANDLE_BUFFER_SIZE];

	} PEB, *PPEB;
	typedef enum _PROCESSINFOCLASS
	{
		ProcessBasicInformation, // q: PROCESS_BASIC_INFORMATION, PROCESS_EXTENDED_BASIC_INFORMATION
		ProcessQuotaLimits, // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
		ProcessIoCounters, // q: IO_COUNTERS
		ProcessVmCounters, // q: VM_COUNTERS, VM_COUNTERS_EX, VM_COUNTERS_EX2
		ProcessTimes, // q: KERNEL_USER_TIMES
		ProcessBasePriority, // s: KPRIORITY
		ProcessRaisePriority, // s: ULONG
		ProcessDebugPort, // q: HANDLE
		ProcessExceptionPort, // s: PROCESS_EXCEPTION_PORT
		ProcessAccessToken, // s: PROCESS_ACCESS_TOKEN
		ProcessLdtInformation, // qs: PROCESS_LDT_INFORMATION // 10
		ProcessLdtSize, // s: PROCESS_LDT_SIZE
		ProcessDefaultHardErrorMode, // qs: ULONG
		ProcessIoPortHandlers, // (kernel-mode only)
		ProcessPooledUsageAndLimits, // q: POOLED_USAGE_AND_LIMITS
		ProcessWorkingSetWatch, // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
		ProcessUserModeIOPL,
		ProcessEnableAlignmentFaultFixup, // s: BOOLEAN
		ProcessPriorityClass, // qs: PROCESS_PRIORITY_CLASS
		ProcessWx86Information,
		ProcessHandleCount, // q: ULONG, PROCESS_HANDLE_INFORMATION // 20
		ProcessAffinityMask, // s: KAFFINITY
		ProcessPriorityBoost, // qs: ULONG
		ProcessDeviceMap, // qs: PROCESS_DEVICEMAP_INFORMATION, PROCESS_DEVICEMAP_INFORMATION_EX
		ProcessSessionInformation, // q: PROCESS_SESSION_INFORMATION
		ProcessForegroundInformation, // s: PROCESS_FOREGROUND_BACKGROUND
		ProcessWow64Information, // q: ULONG_PTR
		ProcessImageFileName, // q: UNICODE_STRING
		ProcessLUIDDeviceMapsEnabled, // q: ULONG
		ProcessBreakOnTermination, // qs: ULONG
		ProcessDebugObjectHandle, // q: HANDLE // 30
		ProcessDebugFlags, // qs: ULONG
		ProcessHandleTracing, // q: PROCESS_HANDLE_TRACING_QUERY; s: size 0 disables, otherwise enables
		ProcessIoPriority, // qs: IO_PRIORITY_HINT
		ProcessExecuteFlags, // qs: ULONG
		ProcessResourceManagement, // ProcessTlsInformation // PROCESS_TLS_INFORMATION
		ProcessCookie, // q: ULONG
		ProcessImageInformation, // q: SECTION_IMAGE_INFORMATION
		ProcessCycleTime, // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
		ProcessPagePriority, // q: PAGE_PRIORITY_INFORMATION
		ProcessInstrumentationCallback, // qs: PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION // 40
		ProcessThreadStackAllocation, // s: PROCESS_STACK_ALLOCATION_INFORMATION, PROCESS_STACK_ALLOCATION_INFORMATION_EX
		ProcessWorkingSetWatchEx, // q: PROCESS_WS_WATCH_INFORMATION_EX[]
		ProcessImageFileNameWin32, // q: UNICODE_STRING
		ProcessImageFileMapping, // q: HANDLE (input)
		ProcessAffinityUpdateMode, // qs: PROCESS_AFFINITY_UPDATE_MODE
		ProcessMemoryAllocationMode, // qs: PROCESS_MEMORY_ALLOCATION_MODE
		ProcessGroupInformation, // q: USHORT[]
		ProcessTokenVirtualizationEnabled, // s: ULONG
		ProcessConsoleHostProcess, // q: ULONG_PTR // ProcessOwnerInformation
		ProcessWindowInformation, // q: PROCESS_WINDOW_INFORMATION // 50
		ProcessHandleInformation, // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since WIN8
		ProcessMitigationPolicy, // s: PROCESS_MITIGATION_POLICY_INFORMATION
		ProcessDynamicFunctionTableInformation,
		ProcessHandleCheckingMode,
		ProcessKeepAliveCount, // q: PROCESS_KEEPALIVE_COUNT_INFORMATION
		ProcessRevokeFileHandles, // s: PROCESS_REVOKE_FILE_HANDLES_INFORMATION
		ProcessWorkingSetControl, // s: PROCESS_WORKING_SET_CONTROL
		ProcessHandleTable, // since WINBLUE
		ProcessCheckStackExtentsMode,
		ProcessCommandLineInformation, // q: UNICODE_STRING // 60
		ProcessProtectionInformation, // q: PS_PROTECTION
		ProcessMemoryExhaustion, // PROCESS_MEMORY_EXHAUSTION_INFO // since THRESHOLD
		ProcessFaultInformation, // PROCESS_FAULT_INFORMATION
		ProcessTelemetryIdInformation, // PROCESS_TELEMETRY_ID_INFORMATION
		ProcessCommitReleaseInformation, // PROCESS_COMMIT_RELEASE_INFORMATION
		ProcessDefaultCpuSetsInformation,
		ProcessAllowedCpuSetsInformation,
		ProcessSubsystemProcess,
		ProcessJobMemoryInformation, // PROCESS_JOB_MEMORY_INFO
		ProcessInPrivate, // since THRESHOLD2 // 70
		ProcessRaiseUMExceptionOnInvalidHandleClose,
		ProcessIumChallengeResponse,
		ProcessChildProcessInformation, // PROCESS_CHILD_PROCESS_INFORMATION
		ProcessHighGraphicsPriorityInformation,
		ProcessSubsystemInformation, // q: SUBSYSTEM_INFORMATION_TYPE // since REDSTONE2
		ProcessEnergyValues, // PROCESS_ENERGY_VALUES, PROCESS_EXTENDED_ENERGY_VALUES
		ProcessActivityThrottleState, // PROCESS_ACTIVITY_THROTTLE_STATE
		ProcessActivityThrottlePolicy, // PROCESS_ACTIVITY_THROTTLE_POLICY
		ProcessWin32kSyscallFilterInformation,
		ProcessDisableSystemAllowedCpuSets,
		ProcessWakeInformation, // PROCESS_WAKE_INFORMATION
		ProcessEnergyTrackingState, // PROCESS_ENERGY_TRACKING_STATE
		ProcessManageWritesToExecutableMemory, // MANAGE_WRITES_TO_EXECUTABLE_MEMORY // since REDSTONE3
		ProcessCaptureTrustletLiveDump,
		ProcessTelemetryCoverage,
		ProcessEnclaveInformation,
		ProcessEnableReadWriteVmLogging, // PROCESS_READWRITEVM_LOGGING_INFORMATION
		ProcessUptimeInformation, // PROCESS_UPTIME_INFORMATION
		ProcessImageSection,
		ProcessDebugAuthInformation, // since REDSTONE4
		ProcessSystemResourceManagement, // PROCESS_SYSTEM_RESOURCE_MANAGEMENT
		ProcessSequenceNumber, // q: ULONGLONG
		MaxProcessInfoClass
	} PROCESSINFOCLASS;
	typedef LONG KPRIORITY;
	typedef NTSTATUS(NTAPI *pNtReadVirtualMemory)
		(
			IN HANDLE               ProcessHandle,
			IN PVOID                BaseAddress,
			OUT PVOID               Buffer,
			IN ULONG                NumberOfBytesToRead,
			OUT PULONG              NumberOfBytesReaded OPTIONAL
			);
	typedef NTSTATUS(WINAPI *pNtQueryInformationProcess)
		(
			_In_      HANDLE           ProcessHandle,
			_In_      PROCESSINFOCLASS ProcessInformationClass,
			_Out_     PVOID            ProcessInformation,
			_In_      ULONG            ProcessInformationLength,
			_Out_opt_ PULONG           ReturnLength
			);

	typedef NTSTATUS(NTAPI *pNtCreateProcessEx)
		(
			OUT PHANDLE     ProcessHandle,
			IN ACCESS_MASK  DesiredAccess,
			IN POBJECT_ATTRIBUTES ObjectAttributes  OPTIONAL,
			IN HANDLE   ParentProcess,
			IN ULONG    Flags,
			IN HANDLE SectionHandle     OPTIONAL,
			IN HANDLE DebugPort     OPTIONAL,
			IN HANDLE ExceptionPort     OPTIONAL,
			IN BOOLEAN  InJob
			);
	typedef PIMAGE_NT_HEADERS(NTAPI *pRtlImageNtHeader)
		(
			IN PVOID                ModuleAddress
			);


	typedef struct _PROCESS_BASIC_INFORMATION {
		NTSTATUS ExitStatus;
		PVOID PebBaseAddress;
		ULONG_PTR AffinityMask;
		KPRIORITY BasePriority;
		ULONG_PTR UniqueProcessId;
		ULONG_PTR InheritedFromUniqueProcessId;
	} PROCESS_BASIC_INFORMATION;
	typedef PROCESS_BASIC_INFORMATION *PPROCESS_BASIC_INFORMATION;

	typedef NTSTATUS(NTAPI *pNtCreateSection)
		(
			OUT PHANDLE             SectionHandle,
			IN ULONG                DesiredAccess,
			IN POBJECT_ATTRIBUTES   ObjectAttributes OPTIONAL,
			IN PLARGE_INTEGER       MaximumSize OPTIONAL,
			IN ULONG                PageAttributess,
			IN ULONG                SectionAttributes,
			IN HANDLE               FileHandle OPTIONAL
			);

	typedef NTSTATUS(NTAPI *pRtlCreateProcessParametersEx)
		(
			_Out_ PRTL_USER_PROCESS_PARAMETERS *pProcessParameters,
			_In_ PUNICODE_STRING ImagePathName,
			_In_opt_ PUNICODE_STRING DllPath,
			_In_opt_ PUNICODE_STRING CurrentDirectory,
			_In_opt_ PUNICODE_STRING CommandLine,
			_In_opt_ PVOID Environment,
			_In_opt_ PUNICODE_STRING WindowTitle,
			_In_opt_ PUNICODE_STRING DesktopInfo,
			_In_opt_ PUNICODE_STRING ShellInfo,
			_In_opt_ PUNICODE_STRING RuntimeData,
			_In_ ULONG Flags // pass RTL_USER_PROC_PARAMS_NORMALIZED to keep parameters normalized
			);

	typedef VOID(WINAPI *pRtlInitUnicodeString)
		(
			_Inout_  PUNICODE_STRING DestinationString,
			_In_opt_ PCWSTR          SourceString
			);

	typedef NTSTATUS(WINAPI *pNtCreateThreadEx)(
		OUT PHANDLE hThread,
		IN ACCESS_MASK DesiredAccess,
		IN LPVOID ObjectAttributes,
		IN HANDLE ProcessHandle,
		IN LPTHREAD_START_ROUTINE lpStartAddress,
		IN LPVOID lpParameter,
		IN BOOL CreateSuspended,
		IN DWORD StackZeroBits,
		IN DWORD SizeOfStackCommit,
		IN DWORD SizeOfStackReserve,
		OUT LPVOID lpBytesBuffer
		);
