<script lang="ts">
  import { onMount } from "svelte";
  import { Button, DatePicker, Snackbar } from "smelte";
  import TimeInput from "../atoms/TimeInput.svelte";
  import TimeZoneInput from "../atoms/TimeZoneInput.svelte";

  const fullFormat: Intl.DateTimeFormatOptions = {
    year: "numeric",
    month: "long",
    day: "numeric",
    weekday: "short",
    hour: "numeric",
    minute: "numeric",
    second: "numeric",
    hour12: false,
  };

  function initSettings() {
    fetch("/time.json")
      .then((res) => res.json())
      .then((json: { time: string }) => {
        date = new Date(json.time);
        time = date.toLocaleTimeString("en-US", { hour12: false });
      });
  }

  function applySettings() {
    fetch("/time.json", {
      method: "POST",
      headers: [["Content-Type", "application/json"]],
      body: JSON.stringify({
        time: datetime,
      }),
    }).then((res) => {
      snackbar.text = `Update settings ${res.ok ? "succeeded" : "failed"}`;
      snackbar.color = res.ok ? "primary" : "error";
      snackbar.show = true;
    });
  }

  onMount(initSettings);

  let snackbar = {
    show: false,
    text: "",
    color: "primary",
  };

  let { timeZone } = new Intl.DateTimeFormat("default", {}).resolvedOptions();
  let date: Date = new Date();
  let time: string = date.toLocaleTimeString("en-US", { hour12: false });

  $: datetime = (() => {
    const d = new Date(date);
    const [hh, mm, ss] = time?.split(":")?.map((t) => parseInt(t, 10)) ?? [];
    d.setHours(hh, mm, ss);
    const defaultDate = Intl.DateTimeFormat("en-US", fullFormat).format(d);
    const modifiedDate = Intl.DateTimeFormat("en-US", {
      timeZone,
      ...fullFormat,
    }).format(d);
    const timeZoneOffset =
      new Date(defaultDate).getTime() - new Date(modifiedDate).getTime();
    return new Date(d.getTime() + timeZoneOffset);
  })();
</script>

<DatePicker bind:value={date} />
<TimeInput bind:time />
<TimeZoneInput bind:timeZone />

<div class="float-right">
  <Button on:click={applySettings}>Apply</Button>
  <Button color="secondary" on:click={initSettings}>Reset</Button>
</div>

<Snackbar color={snackbar.color} bind:value={snackbar.show}>
  <div>{snackbar.text}</div>
</Snackbar>
