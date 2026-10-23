"""Time-series datasets: construction, XML round trip, THMZ entry naming.

The dataset is the per-timestep environment a transient boundary reads, and its
time axis sets the simulation clock.
"""

import pytest

import pylibraryfemtherm as fem

TOL = 1e-9


def _dataset() -> fem.TimeSeriesData:
    data = fem.TimeSeriesData()
    data.uuid = "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"
    data.name = "Two steps of constant weather"
    data.axis.month = 3
    data.axis.day = 15
    data.axis.hour = 6
    data.axis.minute = 30
    data.axis.step_seconds = 1800.0
    data.series = [
        fem.Series(fem.SeriesRole.AirTemperature, [20.0, 21.0]),
        fem.Series(fem.SeriesRole.RelativeHumidity, [0.5, 0.55]),
    ]
    return data


class TestConstruction:
    def test_defaults(self):
        data = fem.TimeSeriesData()
        assert data.name == "Default Name"
        assert data.protected_ is False
        assert data.axis.month == 1
        assert data.axis.day == 1
        assert data.axis.hour == 0
        assert data.axis.minute == 0
        assert abs(data.axis.step_seconds - 3600.0) < TOL
        assert data.series == []
        assert data.project_name is None
        assert data.description is None

    def test_steps_is_shortest_series(self):
        data = _dataset()
        assert fem.time_series_steps(data) == 2
        # `series` comes back as a copy; the list has to be assigned, not appended to.
        data.series = data.series + [fem.Series(fem.SeriesRole.WindSpeed, [1.0])]
        assert fem.time_series_steps(data) == 1

    def test_append_to_copy_does_not_change_dataset(self):
        data = _dataset()
        data.series.append(fem.Series(fem.SeriesRole.WindSpeed, [1.0]))
        assert len(data.series) == 2

    def test_role_queries(self):
        data = _dataset()
        assert fem.has_role(data, fem.SeriesRole.AirTemperature)
        assert not fem.has_role(data, fem.SeriesRole.HeatFlux)
        assert fem.values_for_role(data, fem.SeriesRole.RelativeHumidity) == [0.5, 0.55]
        assert fem.values_for_role(data, fem.SeriesRole.HeatFlux) is None
        assert fem.provided_roles(data) == [
            fem.SeriesRole.AirTemperature,
            fem.SeriesRole.RelativeHumidity,
        ]

    def test_role_names_round_trip(self):
        for name in fem.series_role_strings():
            role = fem.series_role_from_string(name)
            assert fem.series_role_to_string(role) == name

    def test_alignment(self):
        first = _dataset()
        second = _dataset()
        assert fem.time_series_aligned(first, second)
        second.axis.step_seconds = 3600.0
        assert not fem.time_series_aligned(first, second)
        assert fem.time_series_alignment_issues([first, second])


class TestLibraryRoundTrip:
    def test_save_and_load_string(self):
        library = fem.TimeSeriesDB()
        library.add(_dataset())
        text = library.save_to_string()
        assert "<TimeSeriesData>" in text
        assert "<StepSeconds>1800</StepSeconds>" in text
        assert "<Role>AirTemperature</Role>" in text

        loaded = fem.TimeSeriesDB()
        loaded.load_from_string(text)
        assert loaded.get_names() == ["Two steps of constant weather"]
        data = loaded.get_by_uuid("aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee")
        assert data is not None
        assert data.axis.month == 3
        assert data.axis.day == 15
        assert data.axis.hour == 6
        assert data.axis.minute == 30
        assert abs(data.axis.step_seconds - 1800.0) < TOL
        assert fem.values_for_role(data, fem.SeriesRole.AirTemperature) == [20.0, 21.0]

    def test_legacy_record_without_axis_reads_hourly(self):
        # Datasets written before the axis existed read as 1 January 00:00, hourly.
        text = (
            "<TimeSeriesData><Version>1</Version><TimeSeries>"
            "<UUID>old-record</UUID><Name>Legacy</Name><Protected>false</Protected>"
            "<Series><Role>AirTemperature</Role><Values>1,2,3</Values></Series>"
            "</TimeSeries></TimeSeriesData>"
        )
        library = fem.TimeSeriesDB()
        library.load_from_string(text)
        data = library.get_by_name("Legacy")
        assert data is not None
        assert data.axis.month == 1
        assert abs(data.axis.step_seconds - 3600.0) < TOL
        assert fem.time_series_steps(data) == 3

    def test_dirty_flag(self):
        library = fem.TimeSeriesDB()
        assert not library.is_dirty()
        library.add(_dataset())
        assert library.is_dirty()


class TestArchiveEntries:
    def test_entry_name(self):
        name = fem.thmz.time_series_entry_name("abc")
        assert name == fem.thmz.TIME_SERIES_DIR + "/abc.xml"
        assert fem.thmz.time_series_entry_name("abc", fem.FileFormat.JSON).endswith(".json")
        assert fem.thmz.entry_name_for_format("Model", fem.FileFormat.XML) == "Model.xml"

    def test_datasets_through_a_zip(self, tmp_path):
        archive = str(tmp_path / "datasets.thmz")
        fem.thmz.write_entries({"Model.xml": "<ThermModel/>"}, archive)
        written = fem.save_datasets([_dataset()], archive)
        assert written == 1

        datasets = fem.load_datasets(archive)
        assert len(datasets) == 1
        assert datasets[0].name == "Two steps of constant weather"

        entries = fem.thmz.read_entries(archive)
        assert fem.thmz.time_series_entry_name(_dataset().uuid) in entries
        from_entries = fem.load_datasets_from_entries(entries)
        assert len(from_entries) == 1
        assert fem.values_for_role(from_entries[0], fem.SeriesRole.RelativeHumidity) == [0.5, 0.55]


def _epw_text(dry_bulbs: list[str]) -> str:
    """A weather file of a few hourly records from 1 January: 35 fields each, the ones a
    dataset reads set, the rest zero."""
    header = "LOCATION,Sydney,NSW,AUS,TMY,947670,-33.95,151.18,10.0,3.0\n" + "HEADER LINE\n" * 7
    records = []
    for hour, dry_bulb in enumerate(dry_bulbs):
        fields = ["0"] * 35
        fields[0:4] = ["2026", "1", "1", str(hour + 1)]
        fields[6] = dry_bulb
        fields[8] = "50"
        fields[9] = "101325"
        fields[12] = "349.28"
        fields[20] = "180"
        fields[21] = "3"
        records.append(",".join(fields))
    return header + "\n".join(records) + "\n"


CSV_TEXT = "Time,Air Temperature,RelativeHumidity,Comfort Index\n2026-01-01 00:00,21.3,0.52,5\n2026-01-01 01:00,21.1,0.55,6\n"


class TestSeriesValues:
    def test_series_values_is_the_non_optional_form(self):
        assert fem.series_values(_dataset(), fem.SeriesRole.AirTemperature) == [20.0, 21.0]
        assert fem.values_for_role(_dataset(), fem.SeriesRole.WindSpeed) is None
        with pytest.raises(KeyError, match="Two steps of constant weather carries no WindSpeed series"):
            fem.series_values(_dataset(), fem.SeriesRole.WindSpeed)


class TestKeywordConstruction:
    def test_axis_and_dataset_by_keyword(self):
        axis = fem.TimeAxis(month=3, day=15, hour=6, minute=30, step_seconds=1800.0)
        data = fem.TimeSeriesData(
            name="Two steps", axis=axis, series=[fem.Series(fem.SeriesRole.AirTemperature, [20.0, 21.0])],
            source="Stated", description="a test")
        assert data.axis == axis
        assert data.name == "Two steps"
        assert data.source == "Stated"
        assert data.uuid == ""
        assert data.protected_ is False
        assert fem.time_series_steps(data) == 2
        assert repr(axis) == "TimeAxis(month=3, day=15, hour=6, minute=30, step_seconds=1800.0)"
        assert repr(data).startswith("TimeSeriesData(name='Two steps', 15 Mar 06:30")
        assert fem.TimeAxis() == fem.TimeAxis(month=1, day=1)

    def test_steps_are_one_length_or_one_per_step(self):
        uneven: fem.TimeAxis = fem.TimeAxis(step_seconds=[900.0, 1800.0, 3600.0])
        assert uneven.step_seconds == [900.0, 1800.0, 3600.0]
        assert repr(uneven) == "TimeAxis(month=1, day=1, hour=0, minute=0, step_seconds=[900.0, 1800.0, 3600.0])"
        # Lengths that are all the same are one length, whichever way they were given.
        assert fem.TimeAxis(step_seconds=[1800.0, 1800.0]) == fem.TimeAxis(step_seconds=1800.0)
        uneven.step_seconds = [600.0, 600.0]
        assert uneven.step_seconds == 600.0
        with pytest.raises(ValueError, match="at least one length"):
            fem.TimeAxis(step_seconds=[])


class TestCreation:
    def test_read_epw_maps_five_roles_and_the_station(self, tmp_path):
        path = tmp_path / "city.epw"
        path.write_text(_epw_text(["21.3", "21.1", "20.8"]))
        result = fem.read_epw(path)
        assert result.data.name == "city"
        assert result.location.city == "Sydney"
        assert abs(result.location.latitude + 33.95) < 1e-9
        assert set(fem.provided_roles(result.data)) == {
            fem.SeriesRole.AirTemperature, fem.SeriesRole.RelativeHumidity, fem.SeriesRole.WindSpeed,
            fem.SeriesRole.WindDirection, fem.SeriesRole.RadiantTemperature,
        }
        assert fem.values_for_role(result.data, fem.SeriesRole.AirTemperature) == [21.3, 21.1, 20.8]
        assert result.data.uuid == fem.content_uuid(result.data)
        assert fem.read_epw_string(_epw_text(["1"]), "probe").data.name == "probe"
        with pytest.raises(ValueError):
            fem.read_epw(tmp_path / "missing.epw")

        zipped = str(tmp_path / "city.zip")
        fem.thmz.write_entries({"city.epw": _epw_text(["21.3", "21.1", "20.8"]), "city.stat": "x"}, zipped)
        assert fem.read_epw(zipped).data == result.data
        assert fem.read_epw(zipped).location.city == "Sydney"

    def test_read_and_write_csv(self, tmp_path):
        result = fem.read_csv_string(CSV_TEXT, "indoor", steps=fem.FixedStep(seconds=3600.0))
        assert result.had_time_column and not result.had_timestep_column
        assert result.ignored_headers == ["Comfort Index"]
        assert fem.values_for_role(result.data, fem.SeriesRole.RelativeHumidity) == [0.52, 0.55]
        # A stamp is the end of its step: the run starts one step before the first stamp.
        assert fem.describe_axis(result.data) == "31 Dec 23:00, step 1 h, 2 rows"
        text = fem.write_csv(result.data)
        assert text.splitlines()[0].startswith("Time,Timestep,")
        assert fem.csv_header_for_role(fem.SeriesRole.AirTemperature) in text.splitlines()[0]
        path = tmp_path / "again.csv"
        fem.write_csv_file(result.data, path)
        again = fem.read_csv(path, fem.StepColumn.Timestep)
        assert again.had_timestep_column
        assert again.data.series == result.data.series
        assert again.data.axis == result.data.axis

    def test_the_time_rule_ends_a_step_at_each_stamp(self):
        result = fem.read_csv_string(CSV_TEXT, "indoor", fem.StepColumn.Time)
        assert result.first_row_was_start
        assert fem.values_for_role(result.data, fem.SeriesRole.AirTemperature) == [21.1]
        assert fem.describe_axis(result.data) == "1 Jan 00:00, step 1 h, 1 rows"
        uneven = fem.read_csv_string(
            "Time,Air Temperature\n2026-01-01 00:00,1\n2026-01-01 00:15,2\n2026-01-01 02:15,3\n",
            "uneven", fem.StepColumn.Time)
        assert not fem.evenly_stepped(uneven.data)
        assert fem.step_lengths(uneven.data) == [900.0, 7200.0]
        assert uneven.data.axis == fem.TimeAxis(step_seconds=[900.0, 7200.0])
        assert fem.describe_axis(uneven.data) == "1 Jan 00:00, steps 15 min to 2 h, 2 rows"
        with pytest.raises(ValueError, match="has no Time column"):
            fem.read_csv_string("Air Temperature\n1\n2\n", "bare", fem.StepColumn.Time)

    def test_the_timestep_column_states_every_step(self):
        result = fem.read_csv_string("dt,Air Temperature\n900,1\n900,2\n1800,3\n", "logger", fem.StepColumn.Timestep)
        assert result.had_timestep_column and not result.first_row_was_start
        assert fem.values_for_role(result.data, fem.SeriesRole.AirTemperature) == [1.0, 2.0, 3.0]
        assert result.data.axis == fem.TimeAxis(step_seconds=[900.0, 900.0, 1800.0])
        assert repr(result.data.axis) == (
            "TimeAxis(month=1, day=1, hour=0, minute=0, step_seconds=[900.0, 900.0, 1800.0])")
        assert repr(fem.FixedStep(900.0)) == "FixedStep(seconds=900.0)"
        with pytest.raises(ValueError, match="has no Timestep column"):
            fem.read_csv_string(CSV_TEXT, "indoor", fem.StepColumn.Timestep)
        with pytest.raises(ValueError, match="must be positive"):
            fem.read_csv_string("Step,Air Temperature\n0,1\n", "zero", fem.StepColumn.Timestep)

    def test_generate_interior_inherits_the_axis(self):
        exterior = fem.read_epw_string(_epw_text(["-5"] * 48), "city").data
        interior = fem.generate_interior(exterior, fem.InteriorStandard.EN_15026_DIN_4108, fem.MoistureLoad.Normal)
        assert interior.axis == exterior.axis
        assert fem.time_series_steps(interior) == 48
        assert fem.has_role(interior, fem.SeriesRole.AirTemperature)
        assert fem.has_role(interior, fem.SeriesRole.RelativeHumidity)
        assert fem.time_series_aligned(interior, exterior)
        no_temperature = fem.TimeSeriesData(series=[fem.Series(fem.SeriesRole.WindSpeed, [1.0] * 48)])
        with pytest.raises(ValueError):
            fem.generate_interior(no_temperature, fem.InteriorStandard.ISO_13788, fem.MoistureLoad.High)

    def test_window_is_a_dataset_of_its_own(self):
        year = fem.read_epw_string(_epw_text([str(hour) for hour in range(72)]), "city").data
        january_second = fem.time_series_window(year, 24, 24)
        assert january_second.axis == fem.TimeAxis(month=1, day=2)
        assert fem.values_for_role(january_second, fem.SeriesRole.AirTemperature)[0] == 24.0
        assert fem.time_series_steps(january_second) == 24
        assert january_second.uuid == fem.content_uuid(january_second) != year.uuid
        assert fem.axis_after(fem.TimeAxis(), 24) == fem.TimeAxis(month=1, day=2)
        with pytest.raises(ValueError):
            fem.time_series_window(year, 60, 24)

    def test_import_datasets_by_extension_and_from_a_zip(self, tmp_path):
        csv_path = tmp_path / "indoor.csv"
        csv_path.write_text(CSV_TEXT)
        outcome = fem.import_datasets(csv_path, fem.FixedStep(3600.0))
        assert [imported.data.name for imported in outcome.datasets] == ["indoor"]
        assert outcome.datasets[0].skipped_columns == ["Comfort Index"]

        zip_path = str(tmp_path / "weather.zip")
        fem.thmz.write_entries({"city.epw": _epw_text(["1", "2"]), "indoor.csv": CSV_TEXT, "city.stat": "x"}, zip_path)
        from_zip = fem.import_datasets(zip_path, steps=fem.StepColumn.Time)
        assert sorted(imported.data.name for imported in from_zip.datasets) == ["city", "indoor"]
        assert from_zip.failed_entries == []
        assert fem.supported_import_extensions() == [".epw", ".csv", ".zip"]
        with pytest.raises(ValueError):
            fem.import_datasets(tmp_path / "notes.txt", fem.StepColumn.Time)


class TestRolePairing:
    def test_missing_roles_pairs_a_record_with_a_dataset(self):
        exchange = fem.SurfaceExchange()
        exchange.relative_humidity = fem.SeriesRole.RelativeHumidity
        exchange.convection = fem.BCConvection()
        exchange.convection.air_temperature = fem.SeriesRole.AirTemperature
        exchange.convection.film_coefficient = 8.0
        exchange.radiation = fem.BCFixedCoefficientRadiation(temperature=fem.SeriesRole.RadiantTemperature, coefficient=4.5)
        record = fem.BoundaryCondition()
        record.data = exchange
        assert fem.required_roles(record) == [
            fem.SeriesRole.RelativeHumidity, fem.SeriesRole.AirTemperature, fem.SeriesRole.RadiantTemperature]
        assert fem.missing_roles(record) == fem.required_roles(record)
        weather = fem.TimeSeriesData(series=[fem.Series(fem.SeriesRole.AirTemperature, [1.0])])
        # The radiant temperature follows the air temperature when the dataset lacks it.
        assert fem.missing_roles(record, weather) == [fem.SeriesRole.RelativeHumidity]
        complete = fem.TimeSeriesData(series=[
            fem.Series(fem.SeriesRole.AirTemperature, [1.0]), fem.Series(fem.SeriesRole.RelativeHumidity, [0.5])])
        assert fem.missing_roles(record, complete) == []

    def test_capability_issues_name_the_feature(self):
        record = fem.BoundaryCondition()
        exchange = fem.SurfaceExchange()
        exchange.convection = fem.BCConvection()
        exchange.convection.air_temperature = fem.SeriesRole.AirTemperature
        record.data = exchange
        steady = fem.capability_issues(record, fem.SimulationEngine.HygroThermFEM, fem.CalculationMode.cmSteadyState)
        assert steady and all(isinstance(issue.feature, str) and isinstance(issue.reason, str) for issue in steady)
        assert not fem.usable(record, fem.SimulationEngine.HygroThermFEM, fem.CalculationMode.cmSteadyState)
        assert fem.usable(record, fem.SimulationEngine.HygroThermFEM, fem.CalculationMode.cmTransient)
        assert fem.capability_issues(record, fem.SimulationEngine.HygroThermFEM, fem.CalculationMode.cmTransient) == []
