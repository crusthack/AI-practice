# Database 프로그래밍 학습 저장소

ADO.NET 기초부터 Entity Framework Core, Dapper, 고급 패턴까지 C#으로 단계적으로 학습하기 위한 .NET 실습 모음입니다.

## 빠른 시작

.NET SDK 8.0 이상이 설치된 환경에서 각 모듈 폴더의 `.csproj`를 빌드합니다.

```powershell
# 예시: 특정 모듈 실행
dotnet run --project Phase1_AdoNet\01_SqlConnection\01_SqlConnection.csproj
```

SQLite 기반 모듈은 별도 서버 없이 동작합니다. SQL Server 기반 모듈은 LocalDB 또는 SQL Server Express를 사용합니다.

```powershell
# LocalDB 인스턴스 시작 (SQL Server 기반 모듈용)
sqllocaldb start MSSQLLocalDB
```

## 문서 구조

| 문서 | 용도 |
| --- | --- |
| `README.md` | 저장소 개요, 빌드 방법, 전체 목차 |
| `CLAUDE.md` | 에이전트 작업 지침 및 컨벤션 |
| `LEARNING_GUIDE.md` | 모듈별 학습 교안, 실습 절차, 확장 과제 |

## 전체 로드맵

| Phase | 폴더 | 학습 주제 |
| --- | --- | --- |
| Phase 1 | `Phase1_AdoNet` | ADO.NET Connection, Command, Reader, DataSet, Transaction |
| Phase 2 | `Phase2_SQLite` | SQLite 파일 DB, WAL 모드, 동시성 |
| Phase 3 | `Phase3_EFCore` | Entity Framework Core Code-First, Migration, 관계 매핑 |
| Phase 4 | `Phase4_Dapper` | Dapper 쿼리, 멀티 매핑, 동적 파라미터 |
| Phase 5 | `Phase5_Patterns` | Repository 패턴, Unit of Work, CQRS 기초 |

## 모듈 목차

### Phase 1: ADO.NET

- `01_SqlConnection` — `SqlConnection` 열기/닫기, `ConnectionString` 구성
- `02_Command_Reader` — `SqlCommand` 파라미터 바인딩, `SqlDataReader` 순회
- `03_DataSet` — `SqlDataAdapter`, `DataSet`, `DataTable` 채우기와 업데이트
- `04_Transactions` — `BeginTransaction`, `Commit`, `Rollback`, 중첩 트랜잭션
- `05_BulkInsert` — `SqlBulkCopy`로 대량 행 삽입, 배치 크기 조정

### Phase 2: SQLite

- `06_SQLite_Basics` — `Microsoft.Data.Sqlite`로 파일 DB 생성, CRUD
- `07_SQLite_WAL` — WAL 저널 모드, 동시 읽기/쓰기, `PRAGMA` 설정
- `08_SQLite_Perf` — 트랜잭션 묶기, 인덱스 효과, `EXPLAIN QUERY PLAN`

### Phase 3: Entity Framework Core

- `09_EFCore_CodeFirst` — DbContext, 엔티티 클래스, `EnsureCreated`
- `10_EFCore_Migrations` — `Add-Migration`, `Update-Database`, 스키마 변경 추적
- `11_EFCore_Relations` — One-to-Many, Many-to-Many, Owned Entity
- `12_EFCore_Queries` — LINQ 쿼리, Eager/Lazy Loading, N+1 문제 해결
- `13_EFCore_Performance` — `AsNoTracking`, `ExecuteUpdate`, `CompileQuery`

### Phase 4: Dapper

- `14_Dapper_Basics` — `Query<T>`, `Execute`, 단순 파라미터 바인딩
- `15_Dapper_MultiMap` — `splitOn` 멀티 매핑, 부모-자식 관계 조회
- `16_Dapper_StoredProc` — 저장 프로시저 호출, 출력 파라미터, `CommandType`

### Phase 5: 고급 패턴

- `17_Repository` — Generic Repository, IRepository 인터페이스
- `18_UnitOfWork` — Unit of Work 패턴, 트랜잭션 범위 관리
- `19_CQRS_Basic` — Command/Query 분리, MediatR 없는 순수 구현
- `20_ConnectionPool` — 커넥션 풀 설정, 최소/최대 크기, 상태 모니터링

## 권장 학습 방식

1. `LEARNING_GUIDE.md`에서 해당 모듈의 목표와 관찰 포인트를 읽습니다.
2. `Program.cs`의 메서드 호출 순서를 따라갑니다.
3. SQL Server Profiler 또는 EF Core 로깅으로 실제 실행 쿼리를 확인합니다.
4. 예외 타입(`SqlException`, `DbUpdateException`)을 의도적으로 유발해 오류 처리를 확인합니다.
5. 확장 과제를 구현하고 단위 테스트로 동작을 검증합니다.
