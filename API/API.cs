using System;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;
using System.Text;
using System.Xml.Linq;

/*
Порядок выполнения функций для генерации (без учета создания/удаления/ошибок при работе и использования force-вариаций функций)
1. congigureLog
2. readSignalFolder
3. readPUMLFolder
4. compareSignals
5. сheckPumlData
6. buildCode
7. buildLibrary

//setOutputLocation - должна быть выполнена до шага 6
*/

namespace Plant2CodeAPI
{
    public enum SIGNAL_TYPE : uint
    {
        INPUT = 0,		///< - Входной сигнал
        INNER,			///< - Внутренний сигнал
        OUTPUT,			///< - Выходной сигнал
        TIMER			///< - Таймер
    }
    ///\brief  - Структура для переноса данных в C# код
    [StructLayout(LayoutKind.Sequential)]
	public struct TransferredStruct
	{
		public IntPtr signalName;///<- Название сигнала
		public IntPtr signalType;/**<- Тип сигнала(String). Таймеры имеют nullptr(предполагается std::uint32_t). 
		                          * Возможные значения: bool, Int8, UInt8, Int16, UInt16, Int32, UInt32 и float**/
        public IntPtr ptr;///<- Указатель на данные	(void)
        [MarshalAs(UnmanagedType.I1)]
        public bool isPtr;///<- Является ли тип указателем
        public uint elementCount;///<- Кол-во элементов(для указателей от 1 и выше, для значений = 1)
        public SIGNAL_TYPE signalGroupe; ///< - Группа сигналов
	}
    [StructLayout(LayoutKind.Sequential)]
    public struct LatexTableDataC
    {
        public string name;
        public string path;
        public IntPtr refs;
        [MarshalAs(UnmanagedType.I1)]
        public bool isLandscape;
        public string sizes;
    };
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct LatexPersonDataC
    {
        public string name;
        public string position;
    }
   
    [StructLayout(LayoutKind.Sequential)]
    public struct LatexPersonDataExpandedC
    {
        public string name;
        public string position;
        public string place;
    }
    [StructLayout(LayoutKind.Sequential)]
    public unsafe struct LatexTitleDataC
    {
        public string fname;
        public string fposition;
        public uint firstLineCount;
        public string sname;
        public string sposition;
        public uint secondLineCount;
        public LatexPersonDataExpandedC military;
        public LatexPersonDataExpandedC approve;
    };
    [StructLayout(LayoutKind.Sequential)]
    public struct LatexDataMainC
    {
        public string documentName;
        public string fullProgramName;
        public string documentNumber;

        public ushort documentVersion;
        public ushort objectCode;

        public string programShort;
        [MarshalAs(UnmanagedType.I1)]
        public bool needMilitary;

        public LatexTitleDataC title;
    };
    [StructLayout(LayoutKind.Sequential)]
    public struct UsedAlgorithm
    {
        public string name;
        public string changedName;
        public string extraData;
    };
    [StructLayout(LayoutKind.Sequential)]
    public struct LatexAlgorithmDataToC
    {
        public string name;
        public string changedName;
        public string extraData;

        public string sizeKoef;

        public byte[] isLandscape;
        public uint elementCount;
        public UsedAlgorithm[] used;
        public uint usedCount;
    };
    [StructLayout(LayoutKind.Sequential)]
    public struct UsedAlgorithmC
    {
        public IntPtr name;
        public IntPtr changedName;
        public IntPtr extraData;
    };
    [StructLayout(LayoutKind.Sequential)]
    public struct LatexAlgorithmDataC
    {
        public IntPtr name;
        public IntPtr changedName;
        public IntPtr extraData;

        public IntPtr sizeKoef;
        public IntPtr isLandscape;
        public uint elementCount;
        public IntPtr used;
        public uint usedCount;
    };

    public class NativeAPI
	{
		private const string dllName =
#if WIN64d
			"Plant2Code64d.dll"
#elif WIN64r
			"Plant2Code64r.dll"
#elif WIN32d
			"Plant2Code32d.dll"
#elif WIN32r
			"Plant2Code32r.dll"
#else
            "Plant2Code.dll"
#endif
            ;

        public delegate void log(string str);
        ///\brief Функция возвращающая объект Wrapper, занимающийся генерацией кода
        ///\return Указатель на объект Wrapper

        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern IntPtr createWrapper();
		///\brief Функция удаляющая объект Wrapper
		///\param[in,out] ptr - Указатель на объект Wrapper
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern void destroyWrapper(IntPtr ptr);
		
		///\brief Функция устанавливающая вывод сообщений от Wrapper`а
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\param[in,out] logFunction - Функция-логгер
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static extern void configureLog(IntPtr ptr, log logFunction);
		///\brief Функция устанавливающая директорию для генерации файлов
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\param[in] path - Путь до директории
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern void setOutputLocation(IntPtr ptr, String path);
        ///\brief Функция считывающая сигналы CSV-таблиц
        ///\param[in,out] ptr - Указатель на объект Wrapper
        ///\param[in] path - Файлы, содержащие сигналы
        ///\param[in] elementCount - Кол-во файлов
        ///\return Возвращает true, если в считывании puml-алгоритмов не было ошибки
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool readSignalFiles(IntPtr ptr, string[] paths, int elementCount);
        ///\brief Функция считывающая Puml-файлы
        ///\param[in,out] ptr - Указатель на объект Wrapper
        ///\param[in] path - Файлы, содержащие puml-алгоритмы
        ///\param[in] elementCount - Кол-во файлов
        ///\return Возвращает true, если в считывании сигналов не было ошибки
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool readPumlFiles(IntPtr ptr, string[] paths, int elementCount);		
		///\brief Функция сверяющая считанные сигналы из CSV-таблиц с сигналами, собранными из puml-файлов (проверка происходит, если предыдущие этапы выполнены успешно)
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если в считывании сигналов не было ошибки
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool compareSignals(IntPtr ptr);
		///\brief Функция сверяющая считанные сигналы из CSV-таблиц с сигналами, собранными из puml-файлов
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если в считывании сигналов не было ошибки
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool forceToCompareSignals(IntPtr ptr);
		///\brief Функция очищающая данные
		///\param[in,out] ptr - Указатель на объект Wrapper
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern void clear(IntPtr ptr);
		///\brief Функция парсинга и проверки puml-алгоритмов (происходит, если предыдущие этапы выполнены успешно)
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если при проверке/парсинге puml-алгоритмов
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool checkPumlData(IntPtr ptr);
		//\brief Функция парсинга и проверки puml-алгоритмов
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если при проверке/парсинге puml-алгоритмов
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool forceToCheckPumlData(IntPtr ptr);
		///\brief Функция генерации кода библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файлов SourceInfo.c (при использовании класса однофайловой генерации и выше), LibInfo.h (при использовании класса двухфайловой генерации и выше), LibInfo.c (при использовании класса двухфайловой генерации и выше) и HeaderInfo.h (при использовании класса библиотечной генерации)
		///\param[in,out] ptr - Указатель на объект Wrapper 
		///\return Возвращает true, если генерация кода выполнена успешно
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool buildCode(IntPtr ptr);
		///\brief Функция генерации кода библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файлов SourceInfo.c (при использовании класса однофайловой генерации и выше), LibInfo.h (при использовании класса двухфайловой генерации и выше), LibInfo.c (при использовании класса двухфайловой генерации и выше) и HeaderInfo.h (при использовании класса библиотечной генерации)
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если генерация кода выполнена успешно
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool forceToBuildCode(IntPtr ptr);
		///\brief Функция генерации dll-библиотеки (происходит, если предыдущие этапы выполнены успешно). Произведет попытку генерации файла outputLib.dll
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если генерация библиотеки
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool buildLibrary(IntPtr ptr);
		///\brief Функция генерации dll-библиотеки. Произведет попытку генерации файла outputLib.dll
		///\param[in,out] ptr - Указатель на объект Wrapper
		///\return Возвращает true, если генерация библиотеки
		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern bool forceToBuildLibrary(IntPtr ptr);



		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern IntPtr getLib(IntPtr ptr);

		[DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
		public static extern void clearString(IntPtr ptr);
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static extern void clearUint8_t(IntPtr ptr);

        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr genUE(IntPtr ptr, ref LatexDataMainC data);
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static unsafe extern IntPtr genLatexTables(IntPtr ptr, LatexTableDataC[] data, uint dataCount);
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr getTablesOutputPath(IntPtr ptr);
        [DllImport(dllName, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr getLatexAlgorithmData(IntPtr ptr, ref uint count);

    }

    public class ControlAPI
	{
        ///\brief Функция получения указателей на существующие переменные
        ///\param[in,out] ptrCount - Указатель (uint32_t) на переменную для записи кол-ва элементов в массиве структур 
        ///\return Массив структур TransferredStruct, содержащих данные о сигналах
        [DllImport("outputLib.dll")]
		public static extern IntPtr getSignalData(ref uint ptrCount);
		
		///\brief Функция очистки выделенной памяти
		[DllImport("outputLib.dll")]
		public static extern void clearSignalData();
		///\brief Основная функция алгоритма
		[DllImport("outputLib.dll")]
		public static extern void oneStepFunction();
        ///\brief Функция обнуления сигнала/ов
        ///\param[in,out] signal - Название сигнала (char*) для сброса или nullptr для сброса ВСЕХ сигналов
		[DllImport("outputLib.dll")]
        public static extern void signalReset(IntPtr signal);

        ///\brief Функция обнуления входного/ых сигнала/ов
        ///\param[in,out] signal - Название сигнала (char*) для сброса или nullptr для сброса ВСЕХ сигналов
		[DllImport("outputLib.dll")]
        public static extern void signalResetInput(IntPtr signal);
    }
}