# CLAUDE.md — practice-Database 에이전트 지침

이 파일은 이 워크스페이스에서 작업하는 에이전트를 위한 지침입니다.

## 워크스페이스 개요

- **주제**: .NET 데이터베이스 프로그래밍
- **언어**: C# 12 (.NET 8)
- **빌드 시스템**: `dotnet CLI` + `.csproj`
- **DB 엔진**: SQLite(파일 기반, 서버 불필요), SQL Server LocalDB(설치 필요)
- **참고 저장소**: `../practice-dotnet` (C# 기본 문법 및 프로젝트 구조 참고)

## 디렉토리 구조

```
practice-Database/
├── Phase1_AdoNet/          # ADO.NET 기초
├── Phase2_SQLite/          # SQLite 실습
├── Phase3_EFCore/          # Entity Framework Core
├── Phase4_Dapper/          # Dapper micro-ORM
├── Phase5_Patterns/        # 고급 패턴
├── README.md
├── CLAUDE.md               # 이 파일
└── LEARNING_GUIDE.md
```

## 새 모듈 추가 방법

1. 해당 Phase 폴더 아래 모듈 폴더를 만듭니다.
2. `dotnet new console -n <ModuleName> -o <ModuleName>` 으로 프로젝트를 생성합니다.
3. 아래 `Program.cs` 초기화 템플릿을 기반으로 작성합니다.
4. 필요한 NuGet 패키지를 추가합니다.

## Phase별 NuGet 패키지

```
Phase 1 (ADO.NET):
  Microsoft.Data.SqlClient

Phase 2 (SQLite):
  Microsoft.Data.Sqlite

Phase 3 (EF Core):
  Microsoft.EntityFrameworkCore
  Microsoft.EntityFrameworkCore.Sqlite       # SQLite Provider
  Microsoft.EntityFrameworkCore.SqlServer    # SQL Server Provider
  Microsoft.EntityFrameworkCore.Tools        # Migrations CLI

Phase 4 (Dapper):
  Dapper
  Microsoft.Data.Sqlite 또는 Microsoft.Data.SqlClient

Phase 5 (Patterns):
  위 패키지 중 필요한 것 선택
```

## Program.cs 초기화 템플릿

```csharp
// Phase 1~2 (ADO.NET / SQLite 기반)
using Microsoft.Data.Sqlite; // 또는 Microsoft.Data.SqlClient

const string ConnectionString = "Data Source=practice.db"; // SQLite
// const string ConnectionString = "Server=(localdb)\\MSSQLLocalDB;Database=PracticeDb;Integrated Security=true";

await using var conn = new SqliteConnection(ConnectionString);
await conn.OpenAsync();

// --- 여기에 구현 ---
```

```csharp
// Phase 3 (EF Core 기반)
using Microsoft.EntityFrameworkCore;

var options = new DbContextOptionsBuilder<AppDbContext>()
    .UseSqlite("Data Source=practice.db")
    .LogTo(Console.WriteLine, LogLevel.Information)
    .Options;

await using var db = new AppDbContext(options);
await db.Database.EnsureCreatedAsync();

// --- 여기에 구현 ---
```

## 코딩 컨벤션

- 모든 DB 작업은 `async`/`await`를 사용합니다 (`ExecuteReaderAsync`, `ExecuteNonQueryAsync` 등).
- `IAsyncDisposable` 또는 `using` 블록으로 커넥션과 커맨드를 반드시 해제합니다.
- SQL 파라미터는 항상 바인딩(@param)을 사용하고 문자열 보간으로 SQL을 조립하지 않습니다.
- EF Core 모듈에서는 `AsNoTracking()`을 읽기 전용 쿼리에 기본으로 사용합니다.
- 각 모듈의 `Program.cs`는 개념을 단계적으로 보여주는 메서드 분리 구조로 작성합니다.

## 프로젝트 파일 기본값 (.csproj)

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net8.0</TargetFramework>
    <Nullable>enable</Nullable>
    <ImplicitUsings>enable</ImplicitUsings>
  </PropertyGroup>
</Project>
```

## Phase별 학습 진행 현황

| Phase | 상태 | 비고 |
| --- | --- | --- |
| Phase 1: ADO.NET | 미시작 | |
| Phase 2: SQLite | 미시작 | |
| Phase 3: EF Core | 미시작 | |
| Phase 4: Dapper | 미시작 | |
| Phase 5: Patterns | 미시작 | |

## 주의 사항

- SQL Server 모듈은 `sqllocaldb start MSSQLLocalDB`로 인스턴스를 먼저 시작해야 합니다.
- SQLite 파일은 프로젝트 실행 디렉토리(`bin/Debug/net8.0/`)에 생성됩니다.
- EF Core Migrations는 프로젝트 폴더에서 `dotnet ef migrations add <Name>`으로 실행합니다.
- `SqlBulkCopy`는 SQL Server 전용입니다. SQLite 대용으로 트랜잭션 묶음 INSERT를 사용합니다.
