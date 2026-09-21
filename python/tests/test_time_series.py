"""Time-series datasets: construction, XML round trip, THMZ entry naming.

The dataset is the per-timestep environment a transient boundary reads, and its
time axis sets the simulation clock.
"""

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
        name = fem.zip.time_series_entry_name("abc")
        assert name == fem.zip.TIME_SERIES_DIR + "/abc.xml"
        assert fem.zip.time_series_entry_name("abc", fem.FileFormat.JSON).endswith(".json")
        assert fem.zip.entry_name_for_format("Model", fem.FileFormat.XML) == "Model.xml"

    def test_datasets_through_a_zip(self, tmp_path):
        archive = str(tmp_path / "datasets.thmz")
        fem.zip.zip_files({"Model.xml": "<ThermModel/>"}, archive)
        written = fem.save_datasets_to_zip_file([_dataset()], archive)
        assert written == 1

        datasets = fem.load_datasets_from_zip_file(archive)
        assert len(datasets) == 1
        assert datasets[0].name == "Two steps of constant weather"

        entries = fem.zip.unzip_files(archive)
        assert fem.zip.time_series_entry_name(_dataset().uuid) in entries
        from_entries = fem.load_datasets_from_entries(entries)
        assert len(from_entries) == 1
        assert fem.values_for_role(from_entries[0], fem.SeriesRole.RelativeHumidity) == [0.5, 0.55]
