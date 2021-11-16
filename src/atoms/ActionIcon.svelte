<script lang="ts">
  import { Icon } from "smelte";
  export let icon: string;
  export let callback: Function;
  let className = "";
	export { className as class };
</script>
<Icon on:click={callback()} class={`cursor-pointer ${className}`}>{icon}</Icon>