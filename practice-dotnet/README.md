---
title: '닷넷 연습'
description: "C# 중심의 닷넷 동작 파악"
---

# 개요
- [MSDN](https://learn.microsoft.com/en-us/dotnet/)

# 프로젝트 설정 
- VS code에서 연습을 합니다. 

## dotnet cli 
```console
// 솔루션 파일 생성 
dotnet new sln -n "PracticeDotnet"
// 프로젝트 생성 
dotnet new console -o "1. CSharpBasics"
// 솔루션에 프로젝트 추가 
dotnet sln add ./"1. CSharpBasics"/"1. CSharpBasics.csproj"
// 프로젝트 빌드 및 실행. 
dotnet build // 솔루션 전체 빌드 
dotnet run --project "1. CSharpBasics"
```

span 정리할 것 