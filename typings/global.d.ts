interface Main {
    compareString: (code1: string, code2: string) => number;
    compareCFG: (code1: string, code2: string, types: string[]) => number;
    formatString: (code: string, types: string[]) => string;
    stackOverflow: (code: string, types: string[]) => string;
    heapOverflow: (code: string, types: string[]) => string;
    intWidthOverflow: (code: string, types: string[]) => string;
    intArithOverflow: (code: string, types: string[]) => string;
    raceCondition: (code: string, types: string[]) => string;
    multithread: (code1: string[], code2: string[], types: string[]) => number[];
}

interface Window {
    main: Main;
}
