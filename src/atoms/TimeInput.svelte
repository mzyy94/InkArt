<script lang="ts">
  import { TextField } from "smelte";

  export let time: string;

  let inputError = "";

  function inputTime(e: Event) {
    const input = e.target as HTMLInputElement;
    const pattern = new RegExp(input.pattern);

    if (pattern.test(input.value)) {
      inputError = "";
      time = input.value;
    } else {
      inputError = "Invalid time format.";
    }
  }
</script>

<TextField
  value={time}
  error={inputError}
  on:change={inputTime}
  append="access_time"
  label="Time"
  pattern="^\d\d:\d\d:\d\d$"
/>
