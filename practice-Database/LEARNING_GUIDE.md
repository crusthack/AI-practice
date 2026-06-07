# Database 프로그래밍 통합 학습 교안

모든 모듈을 순서대로 학습하기 위한 통합 교안입니다. 각 항목은 목표, 관찰 포인트, 핵심 API, 확장 과제로 구성됩니다.

## 공통 실습 절차

1. `Program.cs`에서 메서드 호출 순서를 파악합니다.
2. EF Core 또는 Dapper 모듈은 로깅을 활성화해 실제 실행 SQL을 확인합니다.
3. 의도적으로 잘못된 입력이나 중복 키를 넣어 예외 흐름을 확인합니다.
4. 확장 과제를 구현하고 `dotnet run`으로 결과를 검증합니다.

---

## Phase 1: ADO.NET

### 01_SqlConnection

- **목표**: `SqlConnection`의 열기/닫기 생명 주기와 연결 문자열 구성 요소를 이해합니다.
- **관찰**: `State` 속성이 `Open` → `Closed`로 바뀌는 시점을 확인합니다.
- **핵심 API**: `SqlConnection`, `Open/OpenAsync`, `Close`, `ConnectionString`
- **확장 과제**: 잘못된 연결 문자열로 `SqlException`을 유발하고 오류 코드(`Number`)를 출력합니다.

### 02_Command_Reader

- **목표**: `SqlCommand`로 파라미터화 쿼리를 실행하고 `SqlDataReader`로 결과를 순회합니다.
- **관찰**: `ExecuteReader`와 `ExecuteScalar`, `ExecuteNonQuery`가 반환하는 값의 차이를 확인합니다.
- **핵심 API**: `SqlCommand`, `AddWithValue`, `SqlDataReader`, `Read()`, `GetString`, `GetInt32`
- **확장 과제**: `CommandTimeout`을 1초로 줄이고 `SELECT WAITFOR DELAY '00:00:05'`로 타임아웃을 유발합니다.

### 03_DataSet

- **목표**: `SqlDataAdapter`로 결과를 메모리 내 `DataSet`에 로드하고 변경 후 DB에 반영합니다.
- **관찰**: `DataAdapter.Fill()` 시 실제 SELECT 쿼리가 실행되고, `Update()` 시 INSERT/UPDATE/DELETE가 실행됨을 프로파일러로 확인합니다.
- **핵심 API**: `SqlDataAdapter`, `DataSet`, `DataTable`, `DataRow`, `Update`
- **확장 과제**: `DataTable.GetChanges(DataRowState.Modified)`로 변경된 행만 추려 업데이트합니다.

### 04_Transactions

- **목표**: 트랜잭션으로 여러 SQL 문을 원자적으로 실행합니다.
- **관찰**: 중간에 예외를 강제 발생시켜 `Rollback` 후 데이터가 원상 복구됨을 확인합니다.
- **핵심 API**: `BeginTransaction`, `SqlTransaction.Commit`, `Rollback`, `IsolationLevel`
- **확장 과제**: `IsolationLevel.Serializable`과 `ReadCommitted`의 차이를 두 연결로 실험합니다.

### 05_BulkInsert

- **목표**: `SqlBulkCopy`로 수천 행을 단일 배치로 삽입하고 반복 INSERT 대비 성능을 측정합니다.
- **관찰**: `BatchSize` 설정에 따른 네트워크 왕복 횟수와 실행 시간 변화를 비교합니다.
- **핵심 API**: `SqlBulkCopy`, `WriteToServer`, `BatchSize`, `ColumnMappings`
- **확장 과제**: SQLite에서 트랜잭션 묶음 INSERT로 동일 효과를 구현하고 성능을 비교합니다.

---

## Phase 2: SQLite

### 06_SQLite_Basics

- **목표**: `Microsoft.Data.Sqlite`로 파일 기반 DB를 생성하고 CRUD를 수행합니다.
- **관찰**: DB 파일이 `.db` 확장자로 생성되고 `sqlite3` CLI로 직접 조회 가능함을 확인합니다.
- **핵심 API**: `SqliteConnection`, `SqliteCommand`, `SqliteDataReader`
- **확장 과제**: `:memory:` 연결 문자열로 인메모리 DB를 생성하고 파일 기반과 성능을 비교합니다.

### 07_SQLite_WAL

- **목표**: WAL(Write-Ahead Log) 저널 모드의 동시성 특성을 이해합니다.
- **관찰**: WAL 모드에서 읽기와 쓰기가 서로 블로킹하지 않음을 두 연결로 실험합니다.
- **핵심 API**: `PRAGMA journal_mode=WAL`, `PRAGMA synchronous`, `PRAGMA cache_size`
- **확장 과제**: WAL 모드와 DELETE 모드에서 동시 쓰기 충돌(`SQLITE_BUSY`) 발생 빈도를 비교합니다.

### 08_SQLite_Perf

- **목표**: 인덱스, 트랜잭션 묶음, `EXPLAIN QUERY PLAN`으로 쿼리 성능을 분석하고 개선합니다.
- **관찰**: 인덱스 없는 풀 스캔과 인덱스 스캔의 실행 계획 차이를 `EXPLAIN QUERY PLAN`으로 확인합니다.
- **핵심 API**: `CREATE INDEX`, `EXPLAIN QUERY PLAN`, `BEGIN IMMEDIATE`
- **확장 과제**: 10만 행 삽입 시 트랜잭션 묶음 유무에 따른 시간 차이를 측정합니다.

---

## Phase 3: Entity Framework Core

### 09_EFCore_CodeFirst

- **목표**: 엔티티 클래스와 `DbContext`를 정의하고 `EnsureCreated`로 스키마를 생성합니다.
- **관찰**: 생성된 SQL DDL을 `LogTo`로 출력해 EF Core가 테이블을 어떻게 매핑하는지 확인합니다.
- **핵심 API**: `DbContext`, `DbSet<T>`, `OnModelCreating`, `EnsureCreated`
- **확장 과제**: `[Column]`, `[MaxLength]`, `[Required]` 어노테이션으로 스키마를 제어합니다.

### 10_EFCore_Migrations

- **목표**: 마이그레이션으로 스키마 변경을 추적하고 적용합니다.
- **관찰**: `__EFMigrationsHistory` 테이블에 적용된 마이그레이션 이력이 기록됨을 확인합니다.
- **핵심 CLI**: `dotnet ef migrations add`, `dotnet ef database update`, `dotnet ef migrations list`
- **확장 과제**: 컬럼 추가 → 마이그레이션 → 롤백(`dotnet ef database update <PreviousMigration>`) 흐름을 연습합니다.

### 11_EFCore_Relations

- **목표**: One-to-Many, Many-to-Many 관계를 엔티티로 모델링하고 Include로 조회합니다.
- **관찰**: Many-to-Many에서 EF Core 6+가 생성하는 조인 테이블 SQL을 확인합니다.
- **핵심 API**: `HasMany`, `WithOne`, `HasForeignKey`, `Include`, `ThenInclude`
- **확장 과제**: 자기 참조(계층 구조) 관계를 모델링하고 재귀 조회를 구현합니다.

### 12_EFCore_Queries

- **목표**: LINQ 쿼리가 SQL로 변환되는 과정을 이해하고 N+1 문제를 해결합니다.
- **관찰**: Lazy Loading 활성화 후 루프 내 탐색 프로퍼티 접근 시 N+1 쿼리가 발생함을 로그로 확인합니다.
- **핵심 API**: `Include`, `AsNoTracking`, `AsSplitQuery`, `Select` 프로젝션
- **확장 과제**: `AsSplitQuery` 전후의 SQL 쿼리 수와 실행 계획을 비교합니다.

### 13_EFCore_Performance

- **목표**: `ExecuteUpdate`, `ExecuteDelete`로 변경 추적 없이 대량 업데이트/삭제를 수행합니다.
- **관찰**: 전통적인 `SaveChanges` 방식과 `ExecuteUpdate` 방식의 실행 SQL과 시간을 비교합니다.
- **핵심 API**: `ExecuteUpdateAsync`, `ExecuteDeleteAsync`, `CompileQuery`, `AsNoTracking`
- **확장 과제**: `CompileQuery`로 자주 쓰는 쿼리를 컴파일하고 반복 실행 시간을 측정합니다.

---

## Phase 4: Dapper

### 14_Dapper_Basics

- **목표**: `Query<T>`, `QuerySingle<T>`, `Execute`로 기본 CRUD를 수행합니다.
- **관찰**: 파라미터를 익명 객체로 전달하는 방식과 `DynamicParameters`를 비교합니다.
- **핵심 API**: `Query<T>`, `QuerySingleOrDefault<T>`, `Execute`, `QueryMultiple`
- **확장 과제**: `QueryAsync<T>`로 비동기 버전을 구현하고 동기 버전과 비교합니다.

### 15_Dapper_MultiMap

- **목표**: `splitOn`으로 JOIN 결과를 여러 타입으로 매핑합니다.
- **관찰**: 컬럼명 충돌 시 매핑이 깨지는 케이스를 실험하고 SQL 별칭으로 해결합니다.
- **핵심 API**: `Query<TFirst, TSecond, TReturn>`, `splitOn` 파라미터
- **확장 과제**: 3개 테이블 JOIN 결과를 3-way 멀티 매핑으로 처리합니다.

### 16_Dapper_StoredProc

- **목표**: 저장 프로시저를 호출하고 출력 파라미터를 읽습니다.
- **관찰**: `CommandType.StoredProcedure`와 `CommandType.Text` 모드의 차이를 확인합니다.
- **핵심 API**: `DynamicParameters`, `.Add(direction: ParameterDirection.Output)`, `.Get<T>`
- **확장 과제**: 결과 집합과 출력 파라미터를 동시에 반환하는 저장 프로시저를 작성합니다.

---

## Phase 5: 고급 패턴

### 17_Repository

- **목표**: `IRepository<T>` 인터페이스를 정의하고 EF Core 또는 Dapper 기반으로 구현합니다.
- **관찰**: 상위 레이어가 구체적인 DB 기술에 의존하지 않게 되는 구조를 확인합니다.
- **핵심 개념**: Generic Repository, Specification 패턴
- **확장 과제**: 같은 인터페이스를 EF Core 구현체와 Dapper 구현체로 모두 만들고 교체합니다.

### 18_UnitOfWork

- **목표**: 여러 Repository 작업을 하나의 트랜잭션으로 묶는 Unit of Work를 구현합니다.
- **관찰**: `IUnitOfWork.CommitAsync()`가 단일 `SaveChanges` 또는 트랜잭션 커밋을 수행함을 확인합니다.
- **핵심 개념**: `IUnitOfWork`, 트랜잭션 범위, DI 컨테이너 등록
- **확장 과제**: 커밋 실패 시 모든 변경이 롤백됨을 통합 테스트로 검증합니다.

### 19_CQRS_Basic

- **목표**: Command(쓰기)와 Query(읽기)를 별도 경로로 분리합니다.
- **관찰**: Command는 Unit of Work + EF Core, Query는 Dapper + 읽기 전용 연결을 사용함을 확인합니다.
- **핵심 개념**: ICommand, IQuery, Handler, In-process 메시지 버스 없는 순수 구현
- **확장 과제**: `ICommandHandler<TCommand>` 인터페이스를 정의하고 DI로 핸들러를 등록합니다.

### 20_ConnectionPool

- **목표**: 커넥션 풀 설정 파라미터(`Min Pool Size`, `Max Pool Size`, `Connect Timeout`)를 실험합니다.
- **관찰**: `Max Pool Size`를 1로 줄인 뒤 동시 요청을 보내 `SqlException`(커넥션 풀 고갈)을 유발합니다.
- **핵심 API**: 연결 문자열 파라미터, `SqlConnection.ClearPool`, `ClearAllPools`
- **확장 과제**: `Polly`로 재시도 정책을 추가해 일시적 연결 오류를 투명하게 처리합니다.
