"""The consolidated boundary-condition library: record kinds, Source inputs, XML round
trip, and loading the library THERM embeds in a THMZ.

Uses mullion-jamb-json.thmz, which carries a BoundaryConditions.json entry.
"""

import pytest

import pylibraryfemtherm as fem

TOL = 1e-9


def _prescribed(uuid: str = "p-1") -> fem.BoundaryCondition:
    record = fem.BoundaryCondition()
    record.uuid = uuid
    record.name = "Fixed temperature and humidity"
    state = fem.PrescribedState()
    state.temperature = fem.SeriesRole.PrescribedTemperature
    state.relative_humidity = fem.SeriesRole.PrescribedHumidity
    record.data = state
    return record


def _convective(uuid: str = "c-1") -> fem.BoundaryCondition:
    record = fem.BoundaryCondition()
    record.uuid = uuid
    record.name = "Fixed film, 20 C"
    exchange = fem.SurfaceExchange()
    exchange.relative_humidity = 0.5
    convection = fem.BCConvection()
    convection.air_temperature = 20.0
    convection.film_coefficient = 8.0
    exchange.convection = convection
    record.data = exchange
    return record


def _adiabatic(uuid: str = "a-1") -> fem.BoundaryCondition:
    record = fem.BoundaryCondition()
    record.uuid = uuid
    record.name = "Sealed"
    record.data = fem.NoExchange()
    return record


class TestSources:
    def test_number_becomes_constant(self):
        state = fem.PrescribedState()
        state.temperature = 30.0
        assert isinstance(state.temperature, fem.Constant)
        assert abs(state.temperature.value - 30.0) < TOL
        assert fem.is_constant(state.temperature)
        assert state.relative_humidity is None

    def test_integer_becomes_constant(self):
        state = fem.PrescribedState()
        state.temperature = 30
        assert isinstance(state.temperature, fem.Constant)
        assert abs(state.temperature.value - 30.0) < TOL

    def test_role_becomes_from_time_series(self):
        state = fem.PrescribedState()
        state.relative_humidity = fem.SeriesRole.PrescribedHumidity
        assert isinstance(state.relative_humidity, fem.FromTimeSeries)
        assert state.relative_humidity.role == fem.SeriesRole.PrescribedHumidity
        assert fem.is_from_time_series(state.relative_humidity)

    def test_explicit_wrappers(self):
        convection = fem.BCConvection()
        convection.air_temperature = fem.Constant(21.0)
        convection.film_coefficient = fem.FromTimeSeries(fem.SeriesRole.ConvectiveCoefficient)
        assert abs(convection.air_temperature.value - 21.0) < TOL
        assert convection.film_coefficient.role == fem.SeriesRole.ConvectiveCoefficient
        assert convection.model == fem.BCConvectionModel.Fixed_Convection_Coefficient


class TestRecordKinds:
    def test_prescribed_requires_both_roles(self):
        record = _prescribed()
        assert isinstance(record.data, fem.PrescribedState)
        assert fem.required_roles(record) == [
            fem.SeriesRole.PrescribedTemperature,
            fem.SeriesRole.PrescribedHumidity,
        ]
        assert not fem.is_steady_capable(record)

    def test_constant_exchange_is_steady_capable(self):
        record = _convective()
        assert isinstance(record.data, fem.SurfaceExchange)
        assert fem.required_roles(record) == []
        assert fem.is_steady_capable(record)
        assert fem.is_convective(record)
        assert not fem.is_adiabatic(record)

    def test_no_exchange_is_adiabatic(self):
        record = _adiabatic()
        assert isinstance(record.data, fem.NoExchange)
        assert fem.is_adiabatic(record)
        assert fem.is_steady_capable(record)

    def test_radiation_variants(self):
        exchange = fem.SurfaceExchange()
        black = fem.BCBlackBodyRadiation()
        black.temperature = -5.0
        black.emissivity = 0.9
        exchange.radiation = black
        assert isinstance(exchange.radiation, fem.BCBlackBodyRadiation)
        exchange.radiation = fem.BCAutomaticEnclosure()
        assert isinstance(exchange.radiation, fem.BCAutomaticEnclosure)
        exchange.radiation = None
        assert exchange.radiation is None


class TestLibraryRoundTrip:
    def test_save_and_load_string(self):
        library = fem.BoundaryConditionsDB()
        library.add(_prescribed())
        library.add(_convective())
        library.add(_adiabatic())
        text = library.save_to_string()
        assert "<PrescribedState>" in text
        assert "<FromTimeSeries>PrescribedTemperature</FromTimeSeries>" in text
        assert "<SurfaceExchange>" in text
        assert "<Constant>20</Constant>" in text

        loaded = fem.BoundaryConditionsDB()
        loaded.load_from_string(text)
        assert loaded.get_names() == [
            "Fixed temperature and humidity",
            "Fixed film, 20 C",
            "Sealed",
        ]

        prescribed = loaded.get_by_uuid("p-1")
        assert prescribed is not None
        assert prescribed.data.temperature.role == fem.SeriesRole.PrescribedTemperature

        convective = loaded.get_by_name("Fixed film, 20 C")
        assert convective is not None
        assert abs(convective.data.convection.air_temperature.value - 20.0) < TOL
        assert abs(convective.data.convection.film_coefficient.value - 8.0) < TOL
        assert abs(convective.data.relative_humidity.value - 0.5) < TOL

        sealed = loaded.get_by_uuid("a-1")
        assert sealed is not None
        assert isinstance(sealed.data, fem.NoExchange)

    def test_json_format(self):
        library = fem.BoundaryConditionsDB()
        library.add(_convective())
        text = library.save_to_string(fem.FileFormat.JSON)
        assert text.lstrip().startswith("{")
        loaded = fem.BoundaryConditionsDB()
        loaded.load_from_string(text)
        assert loaded.get_names() == ["Fixed film, 20 C"]

    def test_dirty_flag(self):
        library = fem.BoundaryConditionsDB()
        assert not library.is_dirty()
        library.add(_adiabatic())
        assert library.is_dirty()


class TestEmbeddedLibrary:
    @pytest.fixture()
    def mullion_path(self, test_data_dir):
        import os

        return os.path.join(test_data_dir, "mullion-jamb-json.thmz")

    def test_load_from_entries(self, mullion_path):
        entries = fem.zip.unzip_files(mullion_path)
        assert fem.zip.find_entry(entries, fem.zip.BOUNDARY_CONDITIONS_FILE_NAME)
        library = fem.BoundaryConditionsDB()
        library.load_from_entries(entries)
        names = library.get_names()
        assert "Adiabatic" in names
        adiabatic = library.get_by_name("Adiabatic")
        assert adiabatic is not None
        assert fem.is_steady_capable(adiabatic)

    def test_load_from_zip_file(self, mullion_path):
        library = fem.BoundaryConditionsDB()
        library.load_from_zip_file(mullion_path)
        assert len(library.get_boundary_conditions()) > 0

    def test_archive_without_library_leaves_db_unchanged(self, sample_sill_path):
        library = fem.BoundaryConditionsDB()
        library.add(_adiabatic())
        library.load_from_zip_file(sample_sill_path)
        assert library.get_names() == ["Sealed"]
