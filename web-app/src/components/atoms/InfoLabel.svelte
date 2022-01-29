<script lang="ts">
  export let icon: string;
  export let title: string;
</script>

<section>
  <span>
    <i class="material-icons">{icon}</i>
    {title}:
  </span>
  <span>
    <slot />
  </span>
</section>

<style>
  section {
    width: 100%;
    height: 1em;
    vertical-align: middle;
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    align-items: center;
  }
  i {
    vertical-align: top;
  }
</style>
