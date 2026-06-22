@echo off
cd /d "%~dp0"
chcp 65001 > nul
setlocal enabledelayedexpansion

:: НАСТРОЙКИ ПУТЕЙ
set "EXE_PATH=..\..\executable\CheckNodeCoverage.exe"
set "TESTS_DIR=.\tests"
set "EXPECTED_DIR=.\expected"
set "OUT_DIR=.\tests_output"
set "SUMMARY_FILE=.\tests_summary.txt"

:: Очищаем старый отчет и результаты
if exist "%SUMMARY_FILE%" del "%SUMMARY_FILE%" > nul 2>&1
if exist "%OUT_DIR%" rmdir /s /q "%OUT_DIR%"
mkdir "%OUT_DIR%"

(
echo ==================================================
echo       Функциональное тестирование программы
echo ==================================================
) >> "%SUMMARY_FILE%"

:: Проверка путей
set "ENV_OK=1"
if not exist "%EXE_PATH%"      ( echo [ОШИБКА] Не найден файл программы! & set "ENV_OK=0" )
if not exist "%TESTS_DIR%"     ( echo [ОШИБКА] Папка с тестами не найдена! & set "ENV_OK=0" )
if not exist "%EXPECTED_DIR%"  ( echo [ОШИБКА] Папка с эталонами не найдена! & set "ENV_OK=0" )

if "%ENV_OK%"=="1" (
    set /a total_tests=0
    set /a passed_tests=0

    :: Перебираем ВСЕ .dot файлы в папке tests
    for %%F in ("%TESTS_DIR%\*.dot") do (
        set /a total_tests+=1
        
        (
        echo.
        echo --------------------------------------------------
        echo [ТЕСТ] Проверка: %%~nF.dot
        ) >> "%SUMMARY_FILE%"
        
        :: Запуск программы
        "%EXE_PATH%" "%%F" "%OUT_DIR%\%%~nF_colored.dot" "%OUT_DIR%\%%~nF_report.txt"
        
        if errorlevel 1 (
            (
            echo   [^<-^] КРИТИЧЕСКИЙ СБОЙ: Сбой выполнения программы C++.
            ) >> "%SUMMARY_FILE%"
        ) else (
            set "TEST_OK=1"
            
            :: Проверка раскрашенного DOT
            if exist "%EXPECTED_DIR%\%%~nF_expected.dot" (
                sort "%OUT_DIR%\%%~nF_colored.dot" /O "%OUT_DIR%\%%~nF_colored_sorted.tmp" 2>nul
                sort "%EXPECTED_DIR%\%%~nF_expected.dot" /O "%OUT_DIR%\%%~nF_expected_dot_sorted.tmp" 2>nul
                
                fc /W "%OUT_DIR%\%%~nF_colored_sorted.tmp" "%OUT_DIR%\%%~nF_expected_dot_sorted.tmp" > nul 2>&1
                if errorlevel 1 (
                    (
                    echo   [^<-^] ОШИБКА: Различия в структуре графа.
                    ) >> "%SUMMARY_FILE%"
                    set "TEST_OK=0"
                ) else (
                    (echo   [ОК] Граф совпал с эталоном.) >> "%SUMMARY_FILE%"
                )
                del "%OUT_DIR%\%%~nF_colored_sorted.tmp" > nul 2>&1
                del "%OUT_DIR%\%%~nF_expected_dot_sorted.tmp" > nul 2>&1
            ) else (
                if exist "%OUT_DIR%\%%~nF_colored.dot" (
                    (echo   [^<-^] ОШИБКА: Создан лишний файл .dot!) >> "%SUMMARY_FILE%"
                    set "TEST_OK=0"
                ) else (
                    (echo   [ОК] Выходной .dot файл корректно отсутствует.) >> "%SUMMARY_FILE%"
                )
            )
            
            :: Проверка TXT отчета
            if exist "%EXPECTED_DIR%\%%~nF_expected.txt" (
                sort "%OUT_DIR%\%%~nF_report.txt" /O "%OUT_DIR%\%%~nF_report_sorted.tmp" 2>nul
                sort "%EXPECTED_DIR%\%%~nF_expected.txt" /O "%OUT_DIR%\%%~nF_expected_sorted.tmp" 2>nul
                
                fc /W "%OUT_DIR%\%%~nF_report_sorted.tmp" "%OUT_DIR%\%%~nF_expected_sorted.tmp" > nul 2>&1
                if errorlevel 1 (
                    (
                    echo   [^<-^] ОШИБКА: Различия в составе ошибок.
                    ) >> "%SUMMARY_FILE%"
                    set "TEST_OK=0"
                ) else (
                    (echo   [ОК] Отчет совпал с эталоном.) >> "%SUMMARY_FILE%"
                )
                del "%OUT_DIR%\%%~nF_report_sorted.tmp" > nul 2>&1
                del "%OUT_DIR%\%%~nF_expected_sorted.tmp" > nul 2>&1
            ) else (
                (echo   [ПРЕДУПРЕЖДЕНИЕ] Эталонный файл .txt не найден.) >> "%SUMMARY_FILE%"
                set "TEST_OK=0"
            )
            
            :: Итог по тесту
            if "!TEST_OK!"=="1" (
                set /a passed_tests+=1
                (echo   -^> ТЕСТ ПРОЙДЕН) >> "%SUMMARY_FILE%"
            ) else (
                (echo   -^> ^[^<-^]^ СБОЙ ТЕСТА) >> "%SUMMARY_FILE%"
            )
        )
    )

    (
    echo.
    echo ==================================================
    echo   ИТОГИ: Успешно пройдены !passed_tests! из !total_tests! тестов.
    echo ==================================================
    ) >> "%SUMMARY_FILE%"
)

:: Итог в консоль
echo ==================================================
echo   ТЕСТИРОВАНИЕ ЗАВЕРШЕНО. Результаты: %SUMMARY_FILE%
if defined total_tests (
    echo   Успешно пройдены !passed_tests! из !total_tests! тестов.
)
echo ==================================================

pause
