---
title: "CSharp Basics"
---

# 개요
- 닷넷 C# 프로그램 기본 구성

# 빌드 프로파일 설정. launchSettings
- 닷넷 프로젝트는 빌드시에 명령줄 세팅을 미리 정의한 프로파일을 제작할 수 있습니다. 
- `Properties/launchSettings.json` 파일을 통해 빌드 프로파일을 정의합니다. 
- 프로파일 실행시, `dotnet run --launch-profile <프로파일 이름>`으로 실행 가능합니다. 
```json:properties/launchSettings.json
{
  "$schema": "http://json.schemastore.org/launchsettings.json",
  "profiles": {
    "dev": {
      "commandName": "Project",
      "dotnetRunMessages": true,
      "environmentVariables": {
        "DOTNET_ENVIRONMENT": "Development"
      }
    },
    "prod": {
      "commandName": "Project",
      "dotnetRunMessages": true,
      "environmentVariables": {
        "DOTNET_ENVIRONMENT": "Production"
      }
    }
  }
}
```
# 환경변수 설정. appsettings.json 
- 프로그램 실행시 사용하는 여러가지 환경 변수들에 대해 정의하는 파일입니다. 
- 일반적인 환경 변수 모음과, 환경에 따른 `appsettings.<environment>.json`을 각각 설정할 수 있습니다. 
- 환경 변수는 프로그램 빌드/실행 시 명령줄에서 설정 하거나, 빌드 프로파일을 통해 설정할 수 있습니다. 
```json:appsettings.json
{
    "Custom": {
        "message": "hello, appsettings"
    }
}
```
## 환경에 따른 appsettings
- 개발 환경이라면, 일반 `appsettings.json` 내용을 `appsettings.Development.json`파일이 덮어씁니다. 
- 개발환경, 배포환경에 따른 값 설정을 각각 할 수 있어 편리한 개발이 가능합니다. 

## 명령줄에서 환경변수 주입하기 
- `set CUSTOM__MESSAGE="bye, appsetting"` **windows cmd**에서의 환경변수 주입
- `set DOTNET_ENVIRONMENT=` 초기화 
- `export CUSTOM__MESSAGE="bye, appsetting"` **linux**에서의 환경변수 주입
- `unset DOTNET_ENVIRONMENT` 초기화 

## 환경변수 사용하기 

### 1. csproj 수정 
```csproj
  <ItemGroup>
    <None Update="appsettings*.json">
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
```
- 위 코드를 프로젝트 파일에 추가해줍니다. 

### 2. nuget 패키지 설치 
- `dotnet add package Microsoft.Extensions.Configuration`
- `dotnet add package Microsoft.Extensions.Hosting`

### 3. 코드 작성 
```cs
IConfiguration config = new ConfigurationBuilder()
    .SetBasePath(AppContext.BaseDirectory)
    .AddJsonFile("appsettings.json", optional: false, reloadOnChange: true)
    .AddJsonFile($"appsettings.{environment}.json", optional: true, reloadOnChange: true)
    .AddEnvironmentVariables()
    .AddCommandLine(args)
    .Build();

Console.WriteLine(config["Custom:message"]);
```

## 환경변수 우선순위 
- 우선순위 내림차순 
1. Command-line arguments using the Command-line configuration provider.
2. Environment variables using the Environment Variables configuration provider.
3. App secrets when the app runs in the Development environment.
4. appsettings.Environment.json using the JSON configuration provider.
5. appsettings.json using the JSON configuration provider.
6. ChainedConfigurationProvider : Adds an existing IConfiguration as a source.
#